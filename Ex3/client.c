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


#define PORT 20000
#define PATH "/home/bogdan/Desktop/Target/"

int main(int argc, char* argv[]) {
	int client_sock, i, string_size;
	struct sockaddr_in addr; // address for client socket
	struct sockaddr_in serv_addr; // address for server socket
	struct statvfs stat; // used to get the available space
	char msg[256]; // the message that is sent to the server
	int size_of_file;
	unsigned long available_space;
	char file_on_server[23];
	FILE *file_write;
	char size_rsp[256];
	int bytes_left;
	int bytes_recv;
	char file_buffer[BUFSIZ];


	if (argc == 1) {                                        // exit if there is no argument received
		printf("No file name was sent as argument\n");  
		exit(EXIT_FAILURE);
	}
	
	//determine the size of the string needed to store the arguments
	string_size = 0;
	for (i = 1; i < argc; i++) {
		string_size += strlen(argv[i]);
		if (i + 1 < argc) {
			string_size++;
		}
	}

	//store the arguments as a single string 
	for (i = 1; i < argc; i++) {
		strcat(msg, argv[i]);
		if (i + 1 < argc) {
			strcat(msg, " ");
		}
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	while(1) {

		if ((client_sock = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		perror("Client socket create");
		exit(EXIT_FAILURE);
		}

		if (inet_pton (AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
			perror("Client add convert");
			exit(EXIT_FAILURE);
		}

		if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
			perror("Client connection");
			exit(EXIT_FAILURE);
		}

		if (send(client_sock, msg, sizeof(msg), 0) == -1) {
			perror("Client send");
			exit(EXIT_FAILURE);
		}

		if (read(client_sock, &file_on_server, sizeof(file_on_server)) == -1) {
			perror("Client receive file found");
		    exit(EXIT_FAILURE);
		}

		if(strcmp(file_on_server,"The file was found.") == 0) {
			printf("%s\n",file_on_server);

			if (read(client_sock, (int *) &size_of_file, sizeof(size_of_file)) == -1) {
				perror("Client receive file size");
			    exit(EXIT_FAILURE);
			}

			if (size_of_file < 0) {
				printf("Wrong file size sent by server.\n");
			}
			else {
				if (statvfs(PATH, &stat) != 0) {    
					perror("Client statvfs");
					exit(EXIT_FAILURE);
				}

				available_space = stat.f_bsize * stat.f_bavail;

				if (available_space >= (unsigned int) size_of_file) {
					printf("Client has enough space to save the file.\n");
					memset(size_rsp,0,sizeof(size_rsp));
					strcpy(size_rsp,"ok");
					if (send(client_sock, size_rsp, strlen(size_rsp), 0) == -1) {
						perror("Client send size rsp");
						exit(EXIT_FAILURE);
					}
					chdir(PATH);

					//file_write = fopen(msg,"w");
					if((file_write = fopen(msg,"w")) == NULL) {
						perror("Client create file");
						exit(EXIT_FAILURE);
					}
					bytes_left = size_of_file;

					while(((bytes_recv = recv(client_sock, file_buffer, BUFSIZ, 0 )) > 0) && (bytes_left > 0)) {
						fwrite(file_buffer, sizeof(char), bytes_recv, file_write);
						bytes_left -= bytes_recv;
						printf("Client got %d bytes from server.\n",bytes_recv);
					}
					fclose(file_write);
				}
				else {
					printf("Client doesn't have enough space to save the file.\n");
					memset(size_rsp,0,sizeof(size_rsp));
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
		printf("Enter the name of the next file or 'q' to exit.\n");
		scanf("%s", msg);

		if (strcmp(msg,"q") == 0) {
			exit(EXIT_SUCCESS);
		}
	}
	return 0;

}