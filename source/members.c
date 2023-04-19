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

void members(int* LOGGEDIN, int* ID, int msgFromServer, int clientServerCom){
    if(*LOGGEDIN == 0){
        printf("@You must log in to use this command!\n");
        return;
    }
    char groupName[32];
    printf("Group name: ");
    scanf("%32s", groupName);
    GRP groupMembers;
    CSC buf;
    buf.mtype = MSG_GROUP_MEMBERS;
    buf.senderID = *ID;
    strcpy(buf.groupName, groupName);
    msgsnd(clientServerCom, &buf, SIZE, 0);
    int rcv = msgrcv(msgFromServer, &groupMembers, sizeof(int)*65, MSG_GROUP_MEMBERS, IPC_NOWAIT);
    if(rcv == -1){
        sleep(1);
        rcv = msgrcv(msgFromServer, &groupMembers, sizeof(int)*65, MSG_GROUP_MEMBERS, IPC_NOWAIT);
    }
    if(rcv == -1){
        printf("@Error: Couldn't receive group member list from server!\n");
        return;
    }
    if(groupMembers.conf == 0){
        printf("@Could not find group %s!\n", groupName);
    }
    int fd = open("users.txt", O_RDONLY);


    char name[32];
    lseek(fd, 20, SEEK_SET);
    char c = 'a';
    char currentID[8];
    int currentIDint;
    int idx = 0;
    int usersInGroup = 0;
    int r;
    for(int i = 0; i < 64; ++i) usersInGroup += groupMembers.groupmembers[i]; //sum of all online members
    printf("@%d GROUP MEMEBRS:\n", usersInGroup);
    while(1) {
        while(c != '\t') { //reading ID
            r = read(fd, &c, 1);
            strcpy(&currentID[idx++], &c);
            switch(r) {
                case 0:
                    printf("\n");
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
        currentIDint = atoi(currentID);
        c = 'a';
        if(groupMembers.groupmembers[currentIDint-1] == 1){
            do { //reading username from file
                read(fd, &name[idx++], 1);
            } while(name[idx-1] != '\t');
            name[idx-1] = '\0'; //removing last \t character
            printf("%s\t",name);
            idx = 0;
            while(c != '\n') read(fd, &c, 1);
            c = 'a';    
        } else{
            while(c != '\n') read(fd, &c, 1);
            c = 'a';
            continue;
        }
    }
    close(fd);
    return;
}