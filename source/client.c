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

const int SIZE = sizeof(int)*2 + sizeof(char)*352;

void quit(int* shmadr){
    *shmadr = 1;
    return;
}

void commands(int* endadr, int* LOGGEDIN, int* ID, char* USERNAME, int receiveQueueID, int msgFromServer, int clientServerCom){
    char com[64];
    while(1){
        strcpy(com,"");
        if(*endadr == 1) {
                shmdt(endadr);
                shmdt(LOGGEDIN);
                shmdt(USERNAME);
                exit(0);
        }
        scanf("%s", com);
        if(*endadr == 1) {
                shmdt(endadr);
                shmdt(LOGGEDIN);
                shmdt(USERNAME);
                exit(0);
        }
        if(com[0] == '!'){
            if(!strcmp(com, "!quit")) {
                quit(endadr);
                continue;
            }
            if(!strcmp(com, "!help")) {
                help();
                continue;
            }
            if(!strcmp(com, "!groups")) {
                groups();
                continue;
            }
            if(!strcmp(com, "!login")) {
                login(LOGGEDIN, ID, USERNAME, receiveQueueID, msgFromServer);
                continue;
            }
            if(!strcmp(com, "!online")) {
                online(LOGGEDIN, ID, msgFromServer, clientServerCom);
                continue;
            }
            if(!strcmp(com, "!members")) {
                members(LOGGEDIN, ID, msgFromServer, clientServerCom);
                continue;
            }
            if(!strcmp(com, "!groupjoin")) {
                groupjoin(LOGGEDIN, ID, msgFromServer, clientServerCom);
                continue;
            }
            if(!strcmp(com, "!groupleave")) {
                groupleave(LOGGEDIN, ID, msgFromServer, clientServerCom);
                continue;
            }
            if(!strcmp(com, "!usermessage")) {
                usrmessage(LOGGEDIN, ID, USERNAME, msgFromServer, clientServerCom);
                continue;
            }
            if(!strcmp(com, "!groupmessage")) {
                groupmessage(LOGGEDIN, ID, USERNAME, msgFromServer, clientServerCom);
                continue;
            }
            if(!strcmp(com, "!logout")) {
                logout(LOGGEDIN, ID, USERNAME, msgFromServer, clientServerCom);
                continue;
            }
            else {
                printf("@Unrecognized command!\n");
            }

        }
    }
}


int main(int argc, char* argv[]){
    printf("@Chat made by Aleksander Owczarczak (148172)\n\n@Type !help to see the list of commands\n\n\n");

    int quitclient = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    if(quitclient == -1){
        printf("@Initialization Error! Shutting Down...\n");
        exit(0);
    }
    int *end = (int*)shmat(quitclient, NULL, 0);
    *end = 0;

    int loggedSHM = shmget(IPC_PRIVATE, sizeof(int)*2, IPC_CREAT | IPC_EXCL | 0600);
    int usernameSHM = shmget(IPC_PRIVATE, 32, IPC_CREAT | IPC_EXCL | 0600);
    int* LOGGEDIN = (int*)shmat(loggedSHM, NULL, 0); //Shared memory between processes to keep track of logging status
    int* ID = LOGGEDIN + 1;
    char* USERNAME = (char*)shmat(usernameSHM, NULL, 0);
    *LOGGEDIN = 0;
    *ID = 0;
    strcpy(USERNAME,"");
    
    int msgFromServer = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0600); //Private queue between client's instance and server

    key_t queueKey = ftok("inf148172_s.c", 87);
    int clientServerCom = msgget(queueKey, IPC_CREAT | 0600);

    key_t receiveQueueIDKey = ftok("inf148172_s.c", 89); //Queue for sending private queue id to server
    int receiveQueueID = msgget(receiveQueueIDKey, 0600);

    if(receiveQueueID == -1){
        printf("@Error: Cannot establish a connection with the server\n");
        shmdt(end);
        shmdt(LOGGEDIN);
        shmdt(USERNAME);
        shmctl(quitclient, IPC_RMID, NULL);
        shmctl(loggedSHM, IPC_RMID, NULL);
        shmctl(usernameSHM, IPC_RMID, NULL);
        msgctl(msgFromServer, IPC_RMID, NULL);
        exit(0);
    }

    if(fork() == 0) { //Process checking for server
        key_t online = ftok("inf148172_s.c", 10);
        int shmonline = shmget(online, sizeof(int), 0600);
        if(shmonline == -1){
            printf("@Error: Cannot establish a connection with the server. Shutting down...\n");
            quit(end);
            exit(0);
        }
        int *buf = (int*)shmat(shmonline, NULL, 0);
        while(*end != 1){
            while(*buf == 1 && *end != 1);
            if(*end != 1){
                printf("@Lost Connection with the server! Trying to reconnect");
                for(int i = 0; i < 3; ++i){
                    printf(".");
                    sleep(2);
                    if(*buf == 1) break;
                }
                if(*buf == 0){
                    printf("\n@Could not reestablish connection with the server, ending session...\n");
                    shmdt(buf);
                    quit(end);
                    shmdt(end);
                    shmdt(LOGGEDIN);
                    shmdt(USERNAME);
                    exit(0);
                }
                if(*LOGGEDIN == 1){ //Logging out to prevent errors in communication
                    *LOGGEDIN = 0;
                    *ID = 0;
                    strcpy(USERNAME, "");
                    printf("@Please try to relogin!\n");
                }
                
            } else break;
        }
        shmdt(end);
        shmdt(LOGGEDIN);
        shmdt(USERNAME);
        exit(0);
    }

    if(fork() == 0) { //process receiving text messages
        MSG message;
        while(*end != 1){
            int txtrcv = msgrcv(msgFromServer, &message, sizeof(char)*320, (-1 * MSG_RECEIVE), IPC_NOWAIT);
            if(txtrcv != -1){
                if(strcmp(message.groupName, "")) printf("\n>>@[%s]%s: %s\n", message.groupName, message.senderName, message.msg);
                else printf("\n>>@%s: %s\n", message.senderName, message.msg);
            }
        }
        exit(0);
    }

    if(fork() == 0) { //process scanning for commands
        commands(end, LOGGEDIN, ID, USERNAME, receiveQueueID, msgFromServer, clientServerCom);    
    }

    while(1) {
        if(*end==1){
            CSC buf;
            buf.mtype = MSG_LOGOUT;
            buf.senderID = *ID;
            msgsnd(clientServerCom, &buf, SIZE, 0);
            *LOGGEDIN = 0;
            *ID = 0;
            strcpy(USERNAME,"");
            shmdt(end);
            shmdt(LOGGEDIN);
            shmdt(USERNAME);
            printf("@SHUTDOWN IN 10 SECONDS\n");
            sleep(10); //giving all children processes time to end 
            shmctl(quitclient, IPC_RMID, NULL);
            shmctl(loggedSHM, IPC_RMID, NULL);
            shmctl(usernameSHM, IPC_RMID, NULL);
            msgctl(msgFromServer, IPC_RMID, NULL);
            break;
        } 
    }
    return 0;
}