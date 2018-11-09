#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SERVER_NAME   "/server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_NO_OF_SEATS 200

typedef struct msg{
    int name;
    int no_of_seats;
}message;

int main (){
    mqd_t mqserver;   // Queue descriptor
    int seats_taken = 0; // The number of the next seat that will be given to the client
    message msgrcv; //Used to store the message received from client
    char* msg_in[sizeof(msgrcv)];  // Size of message to be received

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = sizeof(msgrcv);
    attr.mq_curmsgs = 0;

    if ((mqserver = mq_open (SERVER_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) { // Create the message queue on which the clients send the request
        perror ("Server mq_open");
        exit (1);
    }

    while(1){
        if (mq_receive (mqserver,(char *) &msgrcv,(int) msg_in, NULL) == -1) {  // Get the seat reservation from the client
            perror ("Server: mq_receive");
            exit (1);
        }

        printf("Received a request for %d seats from %d .\n",msgrcv.no_of_seats,msgrcv.name);
        printf("Currently there are %d seats available.\n",MAX_NO_OF_SEATS - seats_taken);

        if((MAX_NO_OF_SEATS - seats_taken) >= msgrcv.no_of_seats){                        // Check if there are enough seats available
            printf("Reserving %d seats for %d \n",msgrcv.no_of_seats,msgrcv.name);
            seats_taken += msgrcv.no_of_seats;
        }
        else{
            printf("Not enough seats avilable for the request from %d\n", msgrcv.name);
        }
    }
}