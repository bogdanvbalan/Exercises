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

#define MESSAGE_LENGTH 256 //the maximum length of the messages exchanged by server and client

/* Function used to handle the requests*/
void handleRequest(int socket_des, char source_dir[1024]) {
	int read_file;
	int file_size;
	int bytes_sent;
	int file_found = 0;
	off_t file_offset = 0;
	char msg_client[MESSAGE_LENGTH];
	char file_status[MESSAGE_LENGTH];
	char file_buffer[BUFSIZ];
	DIR *dp = NULL;     
	struct dirent *dptr = NULL; // structure to get info on the files
	struct stat file_stats; //the stats of the file requested by client
	
	while (1) {
		/* Get the name of the file from client*/
		if(read(socket_des, &msg_client, sizeof(msg_client)) == -1) {
			perror("Server read request");
			exit(EXIT_FAILURE);
		}
		printf("Received a request for %s file.\n", msg_client);
		if(strcmp(msg_client,"q") == 0) {
			printf("Ended the connection for client\n");
			exit(EXIT_SUCCESS);
		}
		/* Open the directory specified as source*/
		if((dp = opendir(source_dir)) == NULL) {
			perror("Server open dir");
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
				exit(EXIT_FAILURE);
			}

			/* Open the file indicated by client*/
			chdir(source_dir);                              
			if ((read_file = open(msg_client, O_RDONLY)) == - 1) {   
				perror("Server open file");
				exit(EXIT_FAILURE);
			}

			/* Get the size of the file and send it to client*/
			if ((fstat(read_file, &file_stats)) == -1) {  
				perror("Server get file info");
				exit(EXIT_FAILURE);
			}
			file_size = file_stats.st_size;           
			if(send(socket_des,(int *) &file_size, sizeof(file_size), 0) == -1) {
				perror("Server send file size");
				exit(EXIT_FAILURE);
			}

			/* Check if the client is able to accept the file*/
			memset(msg_client,0,MESSAGE_LENGTH);
			if ((read(socket_des, &msg_client, sizeof(msg_client))) == -1) {
				perror("Server read size response");
				exit(EXIT_FAILURE);
			}
			if (strcmp(msg_client,"ok") == 0) {
				printf("File transfer started.\n");
				/* Start sending the file to the client*/
				while (file_size > 0) {
					if ((bytes_sent = sendfile(socket_des, read_file, &file_offset, BUFSIZ)) == -1) {
						perror("Server send file");
						exit(EXIT_FAILURE);
					}
					file_size -= bytes_sent;
				}
				printf("File sent to the client.\n");
			}
			close(read_file);
		}
		else {
			/* If the file was not found, send the status to client and close the connection.*/
			strncpy(file_status, "File not found.", MESSAGE_LENGTH);
			send(socket_des, &file_status, sizeof(file_status),0 );
		}
		file_found = 0;
		file_offset = 0;
	}
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

	/* Create a new process for each client connection*/
	while (1) {
		if ((new_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) &address_len)) == - 1) {    //accept the connection to the client
			perror("Server accept");
			exit(EXIT_FAILURE);
		}
		switch (fork()) {
			case -1:
				printf("Can't create child. \n");
				close(new_socket);
				break;
			case 0:
				handleRequest(new_socket, path); 
				exit(EXIT_SUCCESS);
			default:
				close(new_socket);
				break;
		}	
	}
	return 0;
}
