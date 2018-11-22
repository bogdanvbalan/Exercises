#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "common.h"
#include <semaphore.h>
#include <sys/stat.h>

static int seats = MAX_NO_OF_SEATS; 

pthread_mutex_t mutex;    // mutex used to protect the number of seats

int srv_nane = 10;

/* Handle a request from a client*/
void *handleRequests(void *data) {
	char client_name[MESSAGE_LENGTH];
    char server_name[MESSAGE_LENGTH];
	char message[MESSAGE_LENGTH];
    struct mq_attr attr;
	int seats_requested;
	size_t size_of_message = sizeof(message);
	mqd_t client_desc, server_desc;
        
    /* Create a queue which will be used to communicate with the client*/
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = size_of_message;
    attr.mq_curmsgs = 0;

    sprintf(server_name, "/%d", srv_name++);

    if ((server_desc = mq_open(server_name, O_RDWR | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Sever create queue in thread");
        exit(EXIT_FAILURE);
    } 

	/* Copy the name of the client*/
	strcpy(client_name, (char *)data);

	/* Open the client queue*/
    if ((client_desc = mq_open(client_name, O_RDWR)) == -1) {
        perror("Server open client queue");
        exit(EXIT_FAILURE);
    }

    /* Inform client that the server is ready to accept the request*/
    memset(message,0,size_of_message);
    strcpy(message,server_name);
    if ((mq_send(client_desc, message, size_of_message, 0)) == -1) {
        perror("Server send name to client");
        exit(EXIT_FAILURE);
    }

    /* Get the number of seats from the client */
    memset(message, 0, size_of_message);
    if ((mq_receive(server_desc, message, size_of_message, NULL)) == -1) {
            perror("Client receive server status");
            exit(EXIT_FAILURE);
    }

    seats_requested = atoi(message);
    printf("The client requested %d seats.\n",seats_requested);
    printf("There are %d seats available.\n",seats);

    /* Check if the seats can be reserved*/
    pthread_mutex_lock(&mutex);
    memset(message, 0, size_of_message);
    if (seats >= seats_requested) {
        strcpy(message,"OK");
        if ((mq_send(client_desc, message, size_of_message, 0)) == -1) {
            perror("Server send reservation OK");
            exit(EXIT_FAILURE); 
        }
        printf("%d seats reserved for %s.\n", seats_requested, client_name);
        seats -= seats_requested;
        printf("%d seats remaining.\n\n", seats);
    }
    else {
        sprintf(message,"%d", seats);
        if ((mq_send(client_desc, message, size_of_message, 0)) == -1) {
            perror("Server send reservation NOT OK");
            exit(EXIT_FAILURE);
        }
        printf("Not enough seats for %s.\n\n", client_name);
    }
    pthread_mutex_unlock(&mutex);

    if (mq_close(server_desc) == -1) {
        perror("Server mq_close");
        exit(EXIT_FAILURE);
    }

    if (mq_unlink(server_name) == -1) {
        perror("Server mq_unlink");
        exit(EXIT_FAILURE);
    }
}

int main () {
    int seats = MAX_NO_OF_SEATS;
    char client_name[MESSAGE_LENGTH];
    struct mq_attr attr;
    size_t size_of_message = sizeof(client_name);
    mqd_t server_desc;

    /* Set the atributes for server mqueue*/
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = size_of_message;
    attr.mq_curmsgs = 0;

    /* Initialization of the mutexe*/
    pthread_mutex_init(&mutex, NULL);

    /* Create a message queue on which the client will send the name of its own queue*/
    if ((server_desc = mq_open(SERVER_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Server create queue");
        exit(EXIT_FAILURE);
    }

    while (1) {

	    /* Get the name of the client*/
	    memset(client_name, 0, size_of_message);
	    if((mq_receive(server_desc, client_name, size_of_message, NULL)) == -1) {
	        perror("Server receive client name");
	        exit(EXIT_FAILURE);
    	}
    	printf("Received the following message: %s\n",client_name);

    	pthread_t tid;
        char name[MESSAGE_LENGTH];
        strcpy(name, client_name);

    	if((pthread_create(&tid, NULL, handleRequests,(void *) name)) != 0) {
    		perror("Server create thread");
    	}
		if((pthread_detach(tid)) != 0) {
			perror("Server thread detach");
		} 
    }

    if (mq_close(server_desc) == -1) {
        perror("Server mq_close");
        exit(EXIT_FAILURE);
    }

    if (mq_unlink(SERVER_NAME) == -1) {
        perror("Server mq_unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}
