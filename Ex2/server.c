#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SERVER_NAME   "/server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_NO_OF_SEATS 200
#define MESSAGE_LENGTH 256

int main () {
    int seats = MAX_NO_OF_SEATS;
    int seats_requested;
    char client_name[MESSAGE_LENGTH];
    char message[MESSAGE_LENGTH];
    struct mq_attr attr;
    size_t size_of_message = sizeof(client_name);
    mqd_t server_desc, client_desc;

    /* Set the atributes for server mqueue*/
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = size_of_message;
    attr.mq_curmsgs = 0;

    /* Create a message queue on which the client will send the name of its own queue*/
    if ((server_desc = mq_open(SERVER_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Server create queue");
        exit(EXIT_FAILURE);
    }

    while (seats) {

        /* Get the name of the client*/
        memset(client_name, 0, size_of_message);
        if((mq_receive(server_desc, client_name, size_of_message, NULL)) == -1) {
            perror("Server receive client name");
            exit(EXIT_FAILURE);
        }
        printf("Received the following message: %s\n",client_name);

        /* Open the client queue*/
        if ((client_desc = mq_open(client_name, O_RDWR)) == -1) {
            perror("Server open client queue");
            exit(EXIT_FAILURE);
        }

        /* Inform client that the server is ready to accept the request*/
        memset(message,0,size_of_message);
        strcpy(message,"OK");
        if ((mq_send(client_desc, message, size_of_message, 0)) == -1) {
            perror("Server send 'OK' to client");
            exit(EXIT_FAILURE);
        }

        /* Get the number of seats from the client */
        memset(message, 0, size_of_message);
        if ((mq_receive(client_desc, message, size_of_message, NULL)) == -1) {
                perror("Client receive server status");
                exit(EXIT_FAILURE);
        }

        seats_requested = atoi(message);
        printf("The client requested %d seats.\n",seats_requested);
        printf("There are %d seats available.\n",seats);

        /* Check if the seats can be reserved*/
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
