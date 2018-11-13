#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 20000



//Function used to handle the reuquest for each client

void handleRequest(int socket_des) {
	int valread;
	char buffer[1024] = {0};

	valread = read(socket_des, buffer, 1024);
	printf("Received a request for %s file.\n", buffer);

}

int main() {
	int server_fd, new_socket; // socket descriptors for server
	struct sockaddr_in address; // address used on the socket
	size_t address_len = sizeof(address);
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Server socket create.");
		exit(EXIT_FAILURE);
	}

	if (bind(server_fd, (struct sockaddr *) &address, address_len) == -1) {
		perror("Server bind socket to add");
		exit(EXIT_FAILURE);

	}

	if (listen (server_fd, 5) < 0) {
		perror("Server set the socket to listen mode.");
		exit(EXIT_FAILURE);
	}

	while(1) {

		if ((new_socket = accept(server_fd, NULL, NULL)) == - 1) {    //accept the connection to the client
			perror("Server accept");
			exit(EXIT_FAILURE);
		}

		switch(fork()) {

			case -1:
				printf("Can't create child. \n");
				close(new_socket);
				break;
			case 0:
				 handleRequest(new_socket);
				 exit(EXIT_SUCCESS);
			default:
				close(new_socket);
				break;
		}
		
	}

	return 0;
}
