#include <stdio.h> 
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <string.h>
#include <sys/types.h>

#define MAX_SEATS 200

struct mesg_buffer {
    long msg_type; 
    int messid; 
    int seat_number; 
} message; 
  
int main() 
{ 
    key_t key = 1234; // The key used to identify the server
    int msgflg = IPC_CREAT | 0666;
    int msgid; 
  
    if((msgid = msgget(key,msgflg)) < 0){
    	perror("msgget");
    	exit(1);
    }
    else {
    	printf("The message queue was created on server side.\n");
    }
  

    return 0; 
} 
