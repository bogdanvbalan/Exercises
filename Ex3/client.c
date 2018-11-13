#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 20000

int main() {
	int client_sock;
	struct sockaddr_in addr; // address for client socket
	struct sockaddr_in serv_addr; //address for server socket

	char* msg = "Hello";

	if ((client_sock = socket(AF_INET, SOCK_STREAM,0)) < 0) {
		perror("Client socket create");
		exit(EXIT_FAILURE);
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if (inet_pton (AF_INET, "127.0.0.1", &serv_addr.sin_addr) <=0){
		perror("Client add convert");
		exit(EXIT_FAILURE);
	}

	if (connect(client_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("Client connection");
		exit(EXIT_FAILURE);
	}

	send( client_sock, msg, strlen(msg), 0);

}