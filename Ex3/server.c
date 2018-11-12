#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 20000

int main() {

	int server_fd, new_socket; // socket descriptors for server
	struct sockaddr_in address; // address used on the socket
	size_t address_len = sizeof(address);
	int valread;
	char buffer[1024] = {0};

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Server socket create.");
		exit(EXIT_FAILURE);
	}

	if (bind(server_fd, (struct sockaddr *) &address, address_len) < 0) {
		perror("Server bind socket to add");
		exit(EXIT_FAILURE);

	}

	if (listen (server_fd, 5) < 0) {
		perror("Server set the socket to listen mode.");
		exit(EXIT_FAILURE);
	}

	if ((new_socket = accept(server_fd, (struct  sockaddr *) &address, (socklen_t *) &address_len)) < 0) {
		perror("Server accept");
		exit(EXIT_FAILURE);
	}

	valread = read(new_socket, buffer, 1024);
	printf("Message received: %s", buffer);
	return 0;
}
