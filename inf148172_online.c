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
#include "inf148172_queues.h"
#include "inf148172_commands.h"

void online(int* LOGGEDIN, int* ID, int msgFromServer, int clientServerCom){
    if(*LOGGEDIN == 0){
        printf("@You must log in to use this command!\n");
        return;
    }
    LGD loggedInUsers;
    CSC buf;
    buf.mtype = MSG_ONLINE;
    buf.senderID = *ID;
    msgsnd(clientServerCom, &buf, SIZE, 0);
    int rcv = msgrcv(msgFromServer, &loggedInUsers, sizeof(int)*64, MSG_RECEIVE_ONLINE, IPC_NOWAIT);
    if(rcv == -1){
        sleep(1);
        rcv = msgrcv(msgFromServer, &loggedInUsers, sizeof(int)*64, MSG_RECEIVE_ONLINE, IPC_NOWAIT);
    }
    if(rcv == -1){
        printf("@Error: Couldn't receive logged users list from server!\n");
        return;
    }
    int fd = open("users.txt", O_RDONLY);


    char name[32];
    lseek(fd, 20, SEEK_SET);
    char c = 'a';
    char currentID[8];
    int currentIDint;
    int idx = 0;
    int onlineUsers = 0;
    int r;
    for(int i = 0; i < 64; ++i) onlineUsers += loggedInUsers.loggedusers[i]; //sum of all online members
    printf("@%d USERS ONLINE:\n", onlineUsers);
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
        if(loggedInUsers.loggedusers[currentIDint-1] == 1){
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