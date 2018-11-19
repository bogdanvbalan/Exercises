#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "common.h"

void handleRequest() {
    
}

int main () {
    int seats;
    char message[MESSAGE_LENGTH];
    char client_name[MESSAGE_LENGTH];
    struct mq_attr attr;
    size_t size_of_message = sizeof(message);
    mqd_t server_desc, client_desc;

    /* Create a queue which will be used to communicate with the server*/
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = size_of_message;
    attr.mq_curmsgs = 0;

    sprintf(client_name, "/%d", getpid());  //create the name of the client based on pid

    if ((client_desc = mq_open(client_name, O_RDWR | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Client create queue");
        exit(EXIT_FAILURE);
    } 

    /* Open server's message queue and send the name*/
    if ((server_desc = mq_open(SERVER_NAME, O_WRONLY)) == -1) {
        perror("Client open server queue");
        exit(EXIT_FAILURE);
    }
    if ((mq_send(server_desc, client_name, size_of_message, 0)) == -1) {
        perror("Client send name");
        exit(EXIT_FAILURE);
    }

    /* Check if server is ready to accept the request*/
    if ((mq_receive(client_desc, message, size_of_message, NULL)) == -1) {
            perror("Client receive server status");
            exit(EXIT_FAILURE);
    }
    if (strcmp(message,"OK") == 0) {
        printf("The server is ready to accept the request.\n");
    }

    /* Send the seats request*/
    printf("Enter the number of seats:\n");
    scanf("%d",&seats);

    memset(message, 0, size_of_message);
    sprintf(message,"%d",seats);

    if ((mq_send(client_desc, message, size_of_message, 0)) == -1) {
        perror("Client send seats request");
        exit(EXIT_FAILURE);
    }

    /* Get the reservation from server*/
    if ((mq_receive(client_desc, message, size_of_message, NULL)) == -1) {
            perror("Client receive server reservation");
            exit(EXIT_FAILURE);
    }
    if (strcmp(message,"OK") == 0) {
        printf("Got reservation for %d seats.\n",seats);
    }
    else {
        printf("Reservation failed, only %s seats are available.\n", message);
    }
    
    if (mq_close(client_desc) == -1) {
        perror("Client mq_close");
        exit(EXIT_FAILURE);
    }

    if (mq_unlink(client_name) == -1) {
        perror("Client mq_unlink");
        exit(EXIT_FAILURE);
    }
}