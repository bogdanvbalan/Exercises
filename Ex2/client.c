#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SERVER_NAME   "/server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10

typedef struct msg {
    int name;
    int no_of_seats;
} message;

int main () {
    char client_name[64];
    mqd_t mqclient, mqserver;   // queue descriptor
    
    message msgsnt; // the message that will be sent
    msgsnt.name = getpid();
    size_t size_client_messsage = sizeof(msgsnt);
    
    char server_response[128];     // used to store the message received from server
    size_t size_server_response = sizeof(server_response);

    sprintf (client_name, "/%d", getpid());

    struct mq_attr attr;  // struct used to set the arguments for mq_open

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = size_server_response;
    attr.mq_curmsgs = 0;

    if ((mqclient = mq_open (client_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {  // Create a queue on which the server can send the response
        perror("Client mq_open");
        exit(1);
    }

    if ((mqserver = mq_open (SERVER_NAME, O_WRONLY)) == -1) {  // Open the server queue to send the request
        perror("Client mq_open on server");
    }

    printf("How many seats you want to reserve ? \n");    // Get the number of seats that will be reserved
    scanf("%d",&msgsnt.no_of_seats);

    if (mq_send(mqserver, (char*) &msgsnt, size_client_messsage, 0) == -1) {
        perror("Client not able to send message to server.\n");
        exit(1);
    }
    if (mq_receive (mqclient, (char *) server_response, size_server_response, NULL) == -1) {  // Read the response from server
            perror("Client: mq_receive");
            exit(1);
    }
    printf("Server response: %s\n",server_response);

    if (mq_close(mqclient) == -1) {
        perror("Client mq_close");
        exit(1);
    }

    if (mq_unlink(client_name) == -1) {
        perror("Client mq_unlink");
        exit(1);
    }
  }