#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 20000

int main() {

	int server_fd; // socket file descriptor for server
	struct sockaddr_in address; // address used on the socket
	size_t address_len = sizeof(address);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Server: socket descriptor failed.");
		exit(EXIT_FAILURE);
	}

	if (bind(server_fd, (struct sockaddr *) &address, address_len) < 0) {
		perror("Server: bind socket to add");
		exit(EXIT_FAILURE);

	}
	return 0;
}