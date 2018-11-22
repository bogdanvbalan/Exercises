#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <semaphore.h>
#include <sys/stat.h>

#define MESSAGE_LENGTH 256 //the maximum length of the messages exchanged by server and client

/* Function used to handle the requests*/
void handleRequest(int socket_des, char source_dir[1024], char sem_name[MESSAGE_LENGTH]) {
	int i, log_index;
	int read_file;
	int file_size;
	int bytes_sent;
	int file_found = 0;
	off_t file_offset = 0;
	int pid_c = getpid();
	char msg_client[MESSAGE_LENGTH];
	char file_status[MESSAGE_LENGTH];
	DIR *dp = NULL;     
	struct dirent *dptr = NULL; // structure to get info on the files
	struct stat file_stats; //the stats of the file requested by client
	char logs [12][MESSAGE_LENGTH];
	FILE *log;
	sem_t *chld_sem;

	if ((log = fopen( "server.log" , "a" )) == NULL) {
		perror("Open server log");
		exit(EXIT_FAILURE);
	}

	/* Open the semaphore*/
	if ((chld_sem = sem_open(sem_name, O_RDWR)) == SEM_FAILED) { // avoid opening a semaphore that exists
		perror("Child open semaphore");
		exit(EXIT_FAILURE);
	}

	while (1) {
		
		/*Log the activity for current client*/
		sem_wait(chld_sem);
		for (i = 0; i < log_index; i++) {
			fprintf(log,"%s \n",logs[i]);
		}
		sem_post(chld_sem);

		/* Reset the log array*/
		for (i = 0; i < log_index; i++) {
			memset(logs[i], 0, MESSAGE_LENGTH);
		}
		log_index = 0;
		sprintf(logs[log_index++],"=====================================================");

		/* Get the name of the file from client*/
		memset(msg_client, 0, MESSAGE_LENGTH);
		if(read(socket_des, &msg_client, sizeof(msg_client)) == -1) {
			perror("Server read request");
			sprintf(logs[log_index++],"%d:Server failed at get name of file.", pid_c);
			exit(EXIT_FAILURE);
		}
		sprintf(logs[log_index++],"%d:Server got '%s' as file name.", pid_c, msg_client);
		printf("Received a request for %s file.\n", msg_client);

		if(strcmp(msg_client,"q") == 0) {
			printf("Ended the connection for client\n");
			exit(EXIT_SUCCESS);
		}

		/* Open the directory specified as source*/
		if((dp = opendir(source_dir)) == NULL) {
			perror("Server open dir");
			sprintf(logs[log_index++],"%d:Server failed at dir open.", pid_c);
			exit(EXIT_FAILURE);
		}

		/* Check if the file is present in the directory*/
		while ((dptr = readdir(dp)) != NULL) {     
			if (strcmp(dptr->d_name,msg_client) == 0) {
				file_found = 1;
			}
		}

		/* If the file is found start the procedure to send it to the client*/
		if (file_found == 1) {
			/* Send the a message to the client indicating that the file is present.*/
			strncpy(file_status,"File found.",MESSAGE_LENGTH);
			if(send(socket_des, &file_status, sizeof(file_status), 0) == -1) {
				perror("Server send file availability");
				sprintf(logs[log_index++],"%d:Server failed at send availability status", pid_c);
				exit(EXIT_FAILURE);
			}
			sprintf(logs[log_index++],"%d:Server sent '%s' to client as file availability", pid_c, file_status);

			/* Open the file indicated by client*/
			chdir(source_dir);                              
			if ((read_file = open(msg_client, O_RDONLY)) == - 1) {   
				perror("Server open file");
				sprintf(logs[log_index++],"%d:Server failed at open file.", pid_c);
				exit(EXIT_FAILURE);
			}

			/* Get the size of the file and send it to client*/
			if ((fstat(read_file, &file_stats)) == -1) {  
				perror("Server get file info");
				sprintf(logs[log_index++],"%d:Server failed at get file stats.", pid_c);
				exit(EXIT_FAILURE);
			}
			file_size = file_stats.st_size;           
			if(send(socket_des,(int *) &file_size, sizeof(file_size), 0) == -1) {
				perror("Server send file size");
				sprintf(logs[log_index++],"%d:Server failed at send size to client.", pid_c);
				exit(EXIT_FAILURE);
			}
			sprintf(logs[log_index++],"%d:Server sent '%d' as file size to client.", pid_c, file_size);

			/* Check if the client is able to accept the file*/
			memset(msg_client,0,MESSAGE_LENGTH);
			if ((read(socket_des, &msg_client, sizeof(msg_client))) == -1) {
				perror("Server read size response");
				sprintf(logs[log_index++],"%d:Server failed at read accept from client.", pid_c);
				exit(EXIT_FAILURE);
			}
			sprintf(logs[log_index++],"%d:Server got '%s' as response for size from client.", pid_c, msg_client);

			if (strcmp(msg_client,"ok") == 0) {
				printf("File transfer started.\n");
				/* Start sending the file to the client*/
				while (file_size > 0) {
					if ((bytes_sent = sendfile(socket_des, read_file, &file_offset, BUFSIZ)) == -1) {
						perror("Server send file");
						sprintf(logs[log_index++],"%d:Server failed during the sending of the file.", pid_c);
						exit(EXIT_FAILURE);
					}
					file_size -= bytes_sent;
				}
				printf("File sent to the client.\n");
				sprintf(logs[log_index++],"%d:File sent to client.", pid_c);
			}
			close(read_file);
		}
		else {
			/* If the file was not found, send the status to client and close the connection.*/
			strncpy(file_status, "File not found.", MESSAGE_LENGTH);
			send(socket_des, &file_status, sizeof(file_status),0 );
			sprintf(logs[log_index++],"%d:Server sent '%s' as file availability to client.", pid_c, file_status);
		}
		file_found = 0;
		file_offset = 0;
		sprintf(logs[log_index++],"=====================================================\n");
	}
	fclose(log);
	shutdown(socket_des, SHUT_RDWR);
	close(socket_des);
}

int main() {
	int server_fd, new_socket, port, i, rf, counter;
	char temp[MESSAGE_LENGTH], path[MESSAGE_LENGTH], server_ip[MESSAGE_LENGTH];
	char temp_char;
	char read_file[6][MESSAGE_LENGTH];
	struct sockaddr_in address; // address used on the socket
	size_t address_len = sizeof(address);
	FILE *config;
	FILE *log;
	sem_t *config_sem;  // semaphore used to protect the config file

	/* Get the configuration from client.cfg*/
	config = fopen("server.cfg", "r");
	counter = 0;
	rf = 0;
	if (config != NULL) {
		while ((temp_char = getc(config)) != EOF) {
			while ((temp_char != '\n') && (temp_char != ' ') && (counter < MESSAGE_LENGTH -1)) {
				temp[counter++] = temp_char;
				if((temp_char = getc(config)) == EOF) {
					break;
				}
			}
			counter = 0;
			strcpy(read_file[rf++],temp);
			memset(temp, 0, MESSAGE_LENGTH);
		}
	}
	for (i = rf - 1; i >= 0; i--) {
		if ((strcmp(read_file[i], "PORT:")) == 0) {
			port = atoi(read_file[i+1]);
		}
		else if ((strcmp(read_file[i], "SOURCE_DIR:")) == 0) {
			strcpy(path, read_file[i+1]);
		}
		else if ((strcmp(read_file[i], "SERVER_IP:")) == 0) {
			strcpy(server_ip, read_file[i+1]);
		}
	}
	fclose(config);

	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (inet_pton (AF_INET, server_ip, &address.sin_addr) <= 0) {
		perror("Server add convert");
		exit(EXIT_FAILURE);
	}

	/* Create the socket, bind it to address and set it in listen mode*/
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Server socket create.");
		exit(EXIT_FAILURE);
	}
	if (bind(server_fd, (struct sockaddr *) &address, address_len) == -1) {
		perror("Server bind socket to add");
		exit(EXIT_FAILURE);

	}
	if (listen (server_fd, 10) < 0) {
		perror("Server set the socket to listen mode.");
		exit(EXIT_FAILURE);
	}

	/* Create the semaphore */
	memset(temp, 0, MESSAGE_LENGTH);
	sprintf(temp, "/%d", getpid()); //set the name based on pid
	sem_unlink(temp);
	if ((config_sem = sem_open(temp, O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, 1)) == SEM_FAILED) { // avoid opening a semaphore that exists
		perror("Create the semaphore");
		exit(EXIT_FAILURE);
	}

	/* Create a new process for each client connection*/
	while (1) {
		if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &address_len)) == - 1) {    //accept the connection to the client
			perror("Server accept");
			exit(EXIT_FAILURE);
		}
		switch (fork()) {
			case -1:
				printf("Can't create child. \n");
				shutdown(new_socket, SHUT_RDWR);
				close(new_socket);
				break;
			case 0:
				handleRequest(new_socket, path, temp); 
				close(server_fd);
				exit(EXIT_SUCCESS);
			default:
				close(new_socket);
				break;
		}	
	}
	return 0;
}
