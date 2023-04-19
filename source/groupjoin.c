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

void groupjoin(int* LOGGEDIN, int *ID, int msgFromServer, int clientServerCom){
    if(*LOGGEDIN == 0){
        printf("@You must log in to use this command!\n");
        return;
    }
    int fd = open("groups.txt", O_RDWR);
    char groupName[32];
    char currentGroupName[32];
    printf("Group name: ");
    scanf("%32s", groupName);
    char c = 'a';
    int r;
    int idx = 0;
    char currentID[8];
    char newMem = 'a';
    int foundGroup = 0;
    CSC member;
    Confirm conf;
    while(c != '\n') read(fd, &c, 1); //skipping first line
    while(1){
        strcpy(currentGroupName, "");
        while(c != '\t') { //reading ID
            r = read(fd, &c, 1);
            strcpy(&currentID[idx++], &c);
            switch(r) {
                case 0:
                    if(foundGroup == 0) printf("@Coulnd't find the specified group!\n");
                    else printf("\n");
                    close(fd);
                    return;
                case -1:
                    perror("Error while reading through users file\n");
                    close(fd);
                    return;
            }
        }
        currentID[idx-1] = '\0';
        idx = 0;
        c = 'a';
        do { //reading groupname from file
            read(fd, &currentGroupName[idx++], 1);
        } while(currentGroupName[idx-1] != '\t');
        currentGroupName[idx-1] = '\0'; //removing last \t character
        if(!strcmp(groupName, currentGroupName)){
            for(int i = 0; i < *ID; i++) read(fd, &c, 1);
            if(c == '1'){
                printf("@You are already a member of this group!\n");
                close(fd);
                return;
            }
            if(c == '0'){
                lseek(fd, -1, SEEK_CUR);
                newMem = '1';
                write(fd, &newMem, 1);
                member.mtype = MSG_GROUP_JOIN;
                member.senderID = *ID;
                strcpy(member.groupName, currentID);
                msgsnd(clientServerCom, &member, SIZE, 0);
                int rcv = msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
                if(rcv == -1){
                    sleep(1);
                    msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
                }
                if(conf.confirmation == 0) {
                    printf("@Error: Couldn't join group!\n");
                    lseek(fd, -1, SEEK_CUR);
                    newMem = '0';
                    write(fd, &newMem, 1);
                } else printf("@Success!\n");
                close(fd);
                return;
            }
        }
        idx = 0;
        while(c != '\n') read(fd, &c, 1);
        c = 'a';    
    }
    close(fd);
    return;
}