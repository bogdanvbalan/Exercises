#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>


int main() {

	int server_fd; // socket file descriptor for server

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("Server: socket failed.");
		exit(EXIT_FAILURE);
	}

	return 0;
}