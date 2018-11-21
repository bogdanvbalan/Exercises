#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define MESSAGE_LENGTH 256 //the maximum length of the messages exchanged by server and client

char files [10][10] = {"nf", "q", "q", "p.jpg", "binary", "x.txt", "a", "aabi.txt", "big.txt", "abc"}; // files that will be requested randomly by the client

int main(int argc, char* argv[]) {
	int i, counter, rf; // used to loop through the number of arguments
	int random_index = 1; 
	int client_sock, port; 
	int current_byte;
	int bytes_left;
	int bytes_recv;
	char file_name[MESSAGE_LENGTH]; 
	char file_on_server[MESSAGE_LENGTH]; // used to store the availability response from server
	char size_rsp[MESSAGE_LENGTH]; // used to store the client response for size of the file
	char file_buffer[BUFSIZ];
	char temp_char;
	char temp [MESSAGE_LENGTH], path[MESSAGE_LENGTH], server_ip[MESSAGE_LENGTH];
	char read_file [6][MESSAGE_LENGTH];
	unsigned long available_space; //used to store the free bytes on disk
	unsigned long size_of_file; 
	struct sockaddr_in serv_addr; 
	struct statvfs stat; // used to get the available space on disk
	FILE *file_write, *config;
	
	/* Get the arguments in file_name*/
	if (argc == 1) { // exit if no argument is received
		printf("No file name was sent as argument\n");  
		exit(EXIT_FAILURE);
	}

	/* Get the arguments and store in file_name*/
	current_byte = 0;
	for (i = 1; i < argc; i++) {
		current_byte += strlen(argv[i]);
	}
	if (current_byte > MESSAGE_LENGTH - 1) {
		printf("Argument too long.\n");
	}
	else {
		for (i = 1; i < argc; i++) { // store the arguments as a single string
			if (current_byte + sizeof(argv[i]) < MESSAGE_LENGTH - 2) {
				strcat(file_name, argv[i]);
			}
			if (i + 1 < argc) {
				strcat(file_name, " ");
			}
		}
	}
    /* Get the configuration from client.cfg*/
	config = fopen("client.cfg", "r");
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

    /* Set port and ipv4 */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	/* Create a socket and connect to server ip address*/
	if ((client_sock = socket(AF_INET, SOCK_STREAM,0)) == -1) {
	perror("Client socket create");
	exit(EXIT_FAILURE);
	}
	if (inet_pton (AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
		perror("Client add convert");
		exit(EXIT_FAILURE);
	}
	if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
		perror("Client connection");
		exit(EXIT_FAILURE);
	}

	/* Send the name received as argument*/
	if (send(client_sock, file_name, sizeof(file_name), 0) == -1) {
		perror("Client send");
		exit(EXIT_FAILURE);
	}

	while (1) {   // client loops until 'q' key is received, the loop exits using a call to exit()

		/* Get the availability of the file*/
		memset(file_on_server, 0, MESSAGE_LENGTH);
		if (read(client_sock, &file_on_server, sizeof(file_on_server)) == -1) {
			perror("Client receive file found");
		    exit(EXIT_FAILURE);
		}

		/* Check if the file was found on server*/
		if (strcmp(file_on_server,"File found.") == 0) {

			/* Get the file size from server*/
			if (read(client_sock, (int *) &size_of_file, sizeof(size_of_file)) == -1) {
				perror("Client receive file size");
			    exit(EXIT_FAILURE);
			}
			if (size_of_file < 0) {
				printf("Wrong file size sent by server.\n");
			}
			else {

				/* Get the space available on disk*/
				if (statvfs(path, &stat) != 0) {    
					perror("Client statvfs");
					exit(EXIT_FAILURE);
				}
				available_space = stat.f_bsize * stat.f_bavail;

				/* Check if there is enough space to save the file and send ok or exit to server*/
				if (available_space >= (unsigned int) size_of_file) {
					strcpy(size_rsp,"ok");
					if (send(client_sock, size_rsp, strlen(size_rsp), 0) == -1) {
						perror("Client send size rsp");
						exit(EXIT_FAILURE);
					}

					chdir(path); // change working position to the directory indicated in PATH

					/* Save the file that is sent by server*/
					if ((file_write = fopen(file_name,"wb")) == NULL) {
						perror("Client create file");
						exit(EXIT_FAILURE);
					}
					bytes_left = size_of_file;
					while ((bytes_left > 0) && ((bytes_recv = recv(client_sock, file_buffer, BUFSIZ, 0 )) > 0)) {
						
						fwrite(file_buffer, sizeof(char), bytes_recv, file_write);
						bytes_left -= bytes_recv;
					}
					fclose(file_write);
					printf("Got file %s from server.\n",file_name);
				}
				else {
					printf("Client doesn't have enough space to save the file.\n");
					strcpy(size_rsp,"exit");
					if (send(client_sock, size_rsp, strlen(size_rsp), 0) == -1) {
						perror("Client send size rsp");
						exit(EXIT_FAILURE);
					}
				}
			}
		}
		else {
			printf("%s\n",file_on_server);
		}

		/* Get the name of the next file*/
		srand(time(NULL) * getpid() * random_index);
		random_index = rand() % 9;
		memset(file_name, 0, MESSAGE_LENGTH);
		strcpy(file_name, files[random_index]);
		printf("Sending request for %s \n", file_name);

		/* Send the name of the new file to the server*/
		if (send(client_sock, file_name, sizeof(file_name), 0) == -1) {
			perror("Client send");
			exit(EXIT_FAILURE);
		}

		if (strcmp(file_name,"q") == 0) {
			send(client_sock, file_name, sizeof(file_name), 0);
			shutdown(client_sock, SHUT_RDWR);
			close(client_sock);
			exit(EXIT_SUCCESS);
		}
	}
}