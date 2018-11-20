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

#define MESSAGE_LENGTH 256 //the maximum length of the messages exchanged by server and client

int main(int argc, char* argv[]) {
	int i; // used to loop through the number of arguments
	int client_sock, port; 
	char temp [128], path[1024], server_ip[512];
	unsigned long size_of_file; 
	int bytes_left;
	int bytes_recv;
	char file_name[MESSAGE_LENGTH]; 
	char file_on_server[MESSAGE_LENGTH]; // used to store the availability response from server
	char size_rsp[MESSAGE_LENGTH]; // used to store the client response for size of the file
	char file_buffer[BUFSIZ];
	unsigned long available_space; //used to store the free bytes on disk
	struct sockaddr_in serv_addr; 
	struct statvfs stat; // used to get the available space on disk
	FILE *file_write, *config;
	
	/* Get the arguments in file_name*/
	if (argc == 1) { // exit if no argument is received
		printf("No file name was sent as argument\n");  
		exit(EXIT_FAILURE);
	}
	for (i = 1; i < argc; i++) { // store the arguments as a single string
		strcat(file_name, argv[i]);
		if (i + 1 < argc) {
			strcat(file_name, " ");
		}
	}
    
    /* Get the configuration from client.cfg*/
	config = fopen("client.cfg", "r");
	while (fscanf(config, "%s", temp) != EOF) {
		if (strcmp(temp, "PORT:") == 0) {
			fscanf(config, "%d", &port);
		}
		else if(strcmp(temp, "SOURCE_DIR:") == 0) {
			fscanf(config, "%s", path);
		}
		else if(strcmp(temp, "SERVER_IP:") == 0) {
			fscanf(config, "%s", server_ip);
		} 
	}

    /* Set port and ipv4 */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	while (1) {   // client loops until 'q' key is received, the loop exits using a call to exit()

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

		/* Send the name of the file to server and receive the availability of the file*/
		if (send(client_sock, file_name, sizeof(file_name), 0) == -1) {
			perror("Client send");
			exit(EXIT_FAILURE);
		}
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
					if ((file_write = fopen(file_name,"w")) == NULL) {
						perror("Client create file");
						exit(EXIT_FAILURE);
					}
					bytes_left = size_of_file;
					while (((bytes_recv = recv(client_sock, file_buffer, BUFSIZ, 0 )) > 0) && (bytes_left > 0)) {

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

		close(client_sock);

		/* Check if the client requests another file*/
		printf("Enter the name of the next file or 'q' to exit.\n");
		scanf("%s", file_name);
		if (strcmp(file_name,"q") == 0) {
			exit(EXIT_SUCCESS);
		}
	}
	return 0;
}