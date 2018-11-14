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

#define PORT 20000
#define SOURCE_DIR "/home/bogdan/Desktop/Source/"



//Function used to handle the request for each client

void handleRequest(int socket_des) {
	int valread, read_file;
	int srv_rsp = -1;
	char file_name[1024] = {0};  // name of the file requested by client
	char *source_dir = SOURCE_DIR;  // the path to source dir
	DIR *dp = NULL;     // directory stream
	struct dirent *dptr = NULL; // structure to get info on the directory 
	struct stat file_stats; //the stats of the file that is open 

	if((valread = read(socket_des, file_name, 1024)) == -1) {
		perror("Server read request");
		exit(EXIT_FAILURE);
	}

	printf("Received a request for %s file.\n", file_name);

	// open the directory indicatead as source
	if((dp = opendir(source_dir)) == NULL) {
		perror("Server open dir");
		exit(EXIT_FAILURE);
	}
	else {
		while ((dptr = readdir(dp)) != NULL) {     //get the entries in the directory
			if (strcmp(dptr->d_name,file_name) == 0) {
				chdir(source_dir);                              // move to the source dir
				if ((read_file = open(file_name, O_RDONLY)) == - 1) {    // open the file indicated by client
					perror("Server open file");
					exit(EXIT_FAILURE);
				}
				if ((fstat(read_file, &file_stats)) == -1) {   // get the stats of the file
					perror("Server get file info");
					exit(EXIT_FAILURE);
				}

				srv_rsp = file_stats.st_size;                  //create the response
				printf("Sending %d\n",srv_rsp);

				if(send(socket_des,(int *) &srv_rsp, sizeof(srv_rsp), 0) == -1) {
					perror("Server send file size");
					exit(EXIT_FAILURE);
				}
				

				close(read_file);
				exit(EXIT_SUCCESS);
			}
		}
	}
	printf("Sending %d\n",srv_rsp);
	send(socket_des,(int *) &srv_rsp, sizeof(srv_rsp),0 );
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
				handleRequest(new_socket); 
				exit(EXIT_SUCCESS);
			default:
				close(new_socket);
				break;
		}
		
	}
	return 0;
}
