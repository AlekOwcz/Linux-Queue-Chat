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

void groupmessage(int* LOGGEDIN, int *ID, char *USERNAME, int msgFromServer, int clientServerCom){
    if(*LOGGEDIN == 0){
        printf("@You must log-in to send messages!\n");
        return;
    }
    char receivingGroup[32];
    char message[256];
    printf("Group name: ");
    fflush(stdin);
    scanf("%s", receivingGroup);
    char nl; //removing newline
    scanf("%c",&nl);
    int fd = open("groups.txt", O_RDONLY);
    char c = 'a';
    while(c != '\n') read(fd, &c, 1); //skipping first line
    c = 'a';
    char tmpname[32];
    int idx = 0;
    int priority;
    while(1) {
        while(c != '\t') { //skipping through ID
            int r = read(fd, &c, 1);
            switch(r) {
                case 0:
                    printf("@No group \"%s\" found!\n", receivingGroup);
                    close(fd);
                    return;
                case -1:
                    perror("Error while reading through group file\n");
                    close(fd);
                    return;
            }
        }
        do { //reading groupname from file
            read(fd, &tmpname[idx++], 1);
        } while(tmpname[idx-1] != '\t');
        tmpname[idx-1] = '\0'; //removing last \t character
        if(!strcmp(tmpname, receivingGroup)){
            char listOfMembers[64];
            read(fd, listOfMembers, *ID);
            if(listOfMembers[(*ID)-1] == '0'){
                printf("@You must be a member to send messages to a group!\n");
                close(fd);
                return;
            }
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
            buf.mtype = MSG_GROUP_MESSAGE;
            buf.senderID = *ID;
            buf.priority = priority;
            strcpy(buf.receiverName, "");
            conf.confirmation = 0;
            strcpy(buf.senderName,USERNAME);
            strcpy(buf.groupName,receivingGroup);
            strcpy(buf.message,message);
            msgsnd(clientServerCom, &buf, SIZE, 0);
            int rcv = msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
            if(rcv == -1){
                sleep(1);
                msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
            }
            if(conf.confirmation == 0) printf("@Error: Couldn't send message!\n");
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