#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<time.h>
#include "queues.h"
#include "commands.h"

void logout(int* LOGGEDIN, int* ID, char* USERNAME, int msgFromServer, int clientServerCom){
    if(*LOGGEDIN == 0) {
        printf("@You're not logged in!");
        return;
    }
    CSC buf;
    Confirm conf;
    buf.mtype = MSG_LOGOUT;
    buf.senderID = *ID;
    conf.confirmation = 0;
    msgsnd(clientServerCom, &buf, SIZE, 0);
    int rcv = msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
    if(rcv == -1){
        sleep(2);
        msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
    }
    if(rcv == -1){
        printf("@Error: Can't contact the server!\n");
        return;
    }
    if(conf.confirmation != 1){
        printf("@Error: Can't confirm logout with server!\n");
        return;
    }
    if(conf.confirmation == 1) printf("@Confirmed Logout with server\n");
    *LOGGEDIN = 0;
    *ID = 0;
    strcpy(USERNAME,"");
    return;
}