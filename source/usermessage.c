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

void usrmessage(int* LOGGEDIN, int *ID, char *USERNAME, int msgFromServer, int clientServerCom){
    if(*LOGGEDIN == 0){
        printf("@You must log-in to send messages!\n");
        return;
    }
    char receiver[32];
    char message[256];
    printf("Receiver name: ");
    fflush(stdin);
    scanf("%s", receiver);
    char nl; //removing newline
    scanf("%c",&nl);
    int fd = open("users.txt", O_RDONLY);
    lseek(fd, 20, SEEK_SET);
    char c;
    char tmpname[32];
    int idx = 0;
    int priority;
    while(1) {
        while(c != '\t') { //skipping through ID
            int r = read(fd, &c, 1);
            switch(r) {
                case 0:
                    printf("@No user \"%s\" found!\n", receiver);
                    close(fd);
                    return;
                case -1:
                    perror("Error while reading through group file\n");
                    close(fd);
                    return;
            }
        }
        do { //reading username from file
            read(fd, &tmpname[idx++], 1);
        } while(tmpname[idx-1] != '\t');
        tmpname[idx-1] = '\0'; //removing last \t character
        if(!strcmp(tmpname, receiver)){
            printf("Message: ");
            fflush(stdin);
            scanf("%[^\n]%*c", message);
            printf("Priority [1-4]: ");
            fflush(stdin);
            scanf("%d", &priority);
            if(priority < 1) priority = 1;
            if(priority > 4) priority = 4;
            CSC buf;
            Confirm conf;
            buf.mtype = MSG_USER_MESSAGE;
            buf.senderID = *ID;
            buf.priority = priority;
            strcpy(buf.groupName, "");
            conf.confirmation = 2;
            strcpy(buf.senderName,USERNAME);
            strcpy(buf.receiverName,receiver);
            strcpy(buf.message,message);
            msgsnd(clientServerCom, &buf, SIZE, 0);
            int rcv = msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
            if(rcv == -1){
                sleep(1);
                msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
            }
            if(conf.confirmation == 0) printf("@Receiving user is not logged in!\n");
            if(conf.confirmation == 2) printf("@Error: Can't confirm if message was delivered!\n");
            close(fd);
            return;
        }
        idx = 0;
        c = 'a';
        while(c != '\n') read(fd, &c, 1);
    }
    close(fd);
    return;
    
}