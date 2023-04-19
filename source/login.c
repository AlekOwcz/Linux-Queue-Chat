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

void login(int* LOGGEDIN, int* ID, char* USERNAME, int receiveQueueID, int msgFromServer) {
    char username[16];
    char password[32];

    if(*LOGGEDIN == 1) {
        printf("@You're already logged in!\n");
        return;
    }
    printf("USERNAME: ");
    fflush(stdin);
    scanf("%s", username);
    int fd = open("users.txt",O_RDONLY);
    if(fd == -1) {
        perror("Error while opening users file");
        exit(1);
    }
    int usernameLength = strlen(username);
    char name[32];
    char pass[32];
    lseek(fd, 20, SEEK_SET);
    char c;
    char currentID[8];
    int idx = 0;
    while(1) {
        while(c != '\t') { //skipping through ID
            int r = read(fd, &c, 1);
            strcpy(&currentID[idx++], &c);
            switch(r) {
                case 0:
                    printf("@No user \"%s\" found!\n", username);
                    close(fd);
                    return;
                case -1:
                    perror("Error while reading through users file\n");
                    exit(1);
            }
        }
        currentID[idx-1] = '\0';
        idx = 0;
        do { //reading username from file
            read(fd, &name[idx++], 1);
        } while(name[idx-1] != '\t');
        name[idx-1] = '\0'; //removing last \t character
        if(usernameLength == strlen(name)) {
            if(!strcmp(username, name)) {
                printf("\nPASSWORD: ");
                fflush(stdin);
                scanf("%s", password);
                printf("\n");
                idx = 0; //reading password
                do {
                    read(fd, &pass[idx++], 1);
                } while(pass[idx-1] != '\n');
                pass[idx-1] = '\0';
                if(!strcmp(password, pass)){
                    printf("@Successfully logged-in!\n");
                    *LOGGEDIN = 1;
                    strcpy(USERNAME,username);
                    *ID = atoi(currentID);

                    //Sending private queue ID to server for receiving messages
                    STC sendQueue;
                    Confirm conf;
                    conf.confirmation = 0;
                    sendQueue.ID = *ID;
                    sendQueue.mtype = MSG_USERS_LOGGED;
                    sendQueue.queue = msgFromServer;
                    msgsnd(receiveQueueID, &sendQueue, 2*sizeof(int), 0);
                    int rcv = msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
                    if(rcv == -1){
                        sleep(1);
                        msgrcv(msgFromServer, &conf, sizeof(int), MSG_CONFIRM, IPC_NOWAIT);
                    }
                    if(conf.confirmation != 1){
                        printf("@Error: Can't confirm login with server!\n");
                        *ID = 0;
                        strcpy(USERNAME, "");
                        *LOGGEDIN = 0;
                        return;
                    }
                    close(fd);
                    return;
                } else {
                    printf("@Wrong password!\n");
                    close(fd);
                    return;
                }
            }
        }
        idx = 0;
        c = 'a';
        while(c != '\n') read(fd, &c, 1);
    }
    close(fd);
    return;
}