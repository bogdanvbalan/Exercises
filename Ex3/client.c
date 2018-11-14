#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/statvfs.h>

#define PORT 20000
#define PATH "/home/bogdan/Desktop/Target/"

int main(int argc, char* argv[]) {
	int client_sock, i, string_size;
	struct sockaddr_in addr; // address for client socket
	struct sockaddr_in serv_addr; // address for server socket
	struct statvfs stat; // used to get the available space
	char *msg; // the message that is sent to the server
	int size_of_file;
	unsigned long available_space;

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

	msg = malloc(string_size);
	msg[0] = '\0';

	//store the arguments as a single string 
	for (i = 1; i < argc; i++) {
		strcat(msg, argv[i]);
		if (i + 1 < argc) {
			strcat(msg, " ");
		}
	}

	if ((client_sock = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		perror("Client socket create");
		exit(EXIT_FAILURE);
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if (inet_pton (AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		perror("Client add convert");
		exit(EXIT_FAILURE);
	}

	if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
		perror("Client connection");
		exit(EXIT_FAILURE);
	}

	if (send(client_sock, msg, strlen(msg), 0) == -1) {
		perror("Client send");
		exit(EXIT_FAILURE);
	}

	if (read(client_sock, (int *) &size_of_file, sizeof(size_of_file)) == -1) {
		perror("Client receive file size");
	    exit(EXIT_FAILURE);
	}

	if (size_of_file == -1) {
		printf("The file was not found on server.\n");
		exit(EXIT_FAILURE);
	}
	else {
		if (statvfs(PATH, &stat) != 0) {    
			perror("Client statvfs");
			exit(EXIT_FAILURE);
		}

		available_space = stat.f_bsize * stat.f_bavail;

		available_space = 1;

		if (available_space >= (unsigned int) size_of_file) {
			printf("Client has enough space to save the file.\n");
		}
		else {
			printf("Client doesn't have enough space to save the file.\n");
		}
	}

}