#include<stdio.h>
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

typedef struct userInfo {
    int usrID;
    char usrNAME[32];
    int logged;
    int communication;
} USER;

typedef struct groupInfo {
    int groupID;
    char groupNAME[32];
    int members[64];
} GROUP;

void initialization(USER* userlist, GROUP* grouplist){
    for(int i = 0; i < 64; ++i) userlist[i].usrID = 0;
    for(int i = 0; i < 16; ++i) {
        grouplist[i].groupID = 0;
        for(int j = 0; j < 64; ++j) grouplist[i].members[j] = 0;
    }
    int fd = open("users.txt", O_RDONLY);
    char c = 'a';
    while(c != '\n') read(fd, &c, 1); //skipping first line
    c = 'a';
    char idbuf[5];
    char namebuf[32];
    int idx = 1;
    int idTmp;
    int charactersRead;
    while(1) {
        strcpy(idbuf, "");
        strcpy(namebuf, ""); //clearing buffers
        charactersRead = read(fd, &c, 1);
        if(charactersRead == 0) break; //checking for end of file
        idbuf[0] = c;
        while(c != '\t') { //scanning id
            read(fd, &idbuf[idx], 1);
            c = idbuf[idx++];
        }
        idbuf[idx-1] = '\0';

        idTmp = atoi(idbuf);
        userlist[idTmp-1].logged = 0;
        userlist[idTmp-1].usrID = idTmp;

        c = 'a';
        idx = 0;
        while(c != '\t') { //scanning name
            read(fd, &namebuf[idx], 1);
            c = namebuf[idx++];
        }
        namebuf[idx-1] = '\0';

        c = 'a';
        idx = 1;
        strcpy(userlist[idTmp-1].usrNAME, namebuf);

        while(c != '\n') read(fd, &c, 1); //skipping password
    }
    close(fd);

    //groups
    fd = open("groups.txt", O_RDONLY);
    c = 'a';
    while(c != '\n') read(fd, &c, 1); //skipping first line
    c = 'a';
    while(1) {
        strcpy(idbuf, "");
        strcpy(namebuf, ""); //clearing buffers
        charactersRead = read(fd, &c, 1);
        if(charactersRead == 0) break; //checking for end of file
        idbuf[0] = c;
        while(c != '\t') { //scanning id
            read(fd, &idbuf[idx], 1);
            c = idbuf[idx++];
        }
        idbuf[idx-1] = '\0';

        idTmp = atoi(idbuf);
        grouplist[idTmp-1].groupID = idTmp;

        c = 'a';
        idx = 0;
        while(c != '\t') { //scanning name
            read(fd, &namebuf[idx], 1);
            c = namebuf[idx++];
        }
        namebuf[idx-1] = '\0';

        c = 'a';
        
        strcpy(grouplist[idTmp-1].groupNAME, namebuf);
        
        int memid = 1;

        for(int l = 0; l < 64; ++l) {
            read(fd, &c, 1);
            if(c == '1'){
                grouplist[idTmp-1].members[memid-1] = 1;
            }
            memid++;
        }
        read(fd, &c, 1); //reading \n
        c = 'a';
    }
    close(fd);
}

void quit(int* shmadr){
    *shmadr = 1;
    return;
}

int main(int argc, char* argv[]){
    printf("@Server made by Aleksander Owczarczak (148172)\n\n@Type !quit to safely exit the server\n\n\n");
    int SHMuserlist = shmget(IPC_PRIVATE, 64*sizeof(USER), IPC_CREAT | IPC_EXCL | 0600);
    int SHMgrouplist = shmget(IPC_PRIVATE, 16*sizeof(GROUP), IPC_CREAT | IPC_EXCL | 0600);
    USER* userlist = (USER*)shmat(SHMuserlist, NULL, 0);
    GROUP* grouplist = (GROUP*)shmat(SHMgrouplist, NULL, 0);
    initialization(userlist, grouplist);

    key_t queueKey = ftok("inf148172_s.c", 87);
    int clientServerCom = msgget(queueKey, IPC_CREAT | 0600); //Queue to receive messages from clients
    key_t receiveQueueIDKey = ftok("inf148172_s.c", 89);
    int receiveQueueID = msgget(receiveQueueIDKey, IPC_CREAT | 0600); //Queue to receive IDs of client queues
    STC queueBuf;

    int quitserver = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0600);
    if(quitserver == -1){
        printf("Initialization Error! Shutting Down...\n");
        exit(0);
    }
    int *end = (int*)shmat(quitserver, NULL, 0);
    *end = 0;


    if(fork() == 0){ //Proces for receiving queue IDs
        while(1){
            int received = msgrcv(receiveQueueID, &queueBuf, 2*sizeof(int), MSG_USERS_LOGGED, IPC_NOWAIT);
            if(received != -1){
                userlist[queueBuf.ID-1].communication = queueBuf.queue;
                userlist[queueBuf.ID-1].logged = 1;
                Confirm conf;
                conf.mtype = MSG_CONFIRM;
                conf.confirmation = 1;
                msgsnd(queueBuf.queue, &conf, sizeof(int), 0);
                printf("@RECEIVED LOG-IN FROM #%d\n", queueBuf.ID);
            }
            if(*end == 1){
                msgctl(receiveQueueID, IPC_RMID, NULL);
                break;
            }
        }
        exit(0);
    }


    if(fork()==0){ //Process informing clients about server's status 
        key_t online = ftok("inf148172_s.c", 10);
        int shmonline = shmget(online, sizeof(int), IPC_CREAT | IPC_EXCL | 0600); //Shared memory signalising that the server is online
        if(shmonline == -1){
            printf("Server error! Shutting down...\n");
            quit(end);
            exit(0);
        }
        int *isOnline = (int*)shmat(shmonline, NULL, 0);
        *isOnline = 1;
        while(*end == 0);
        shmdt(end);
        *isOnline = 0;
        printf("@GOING OFFLINE\n");
        sleep(3); //Waiting for clients to notice that server is offline
        shmdt(isOnline);
        shmctl(shmonline, IPC_RMID, NULL);
        printf("@REMOVING ASSOCIATED SHARED MEMORY\n");
        exit(0);
    }

    if(fork() == 0){ //Main message passing process
        CSC buffer;
        Confirm conf;
        MSG msg;
        LGD loggedUsers;
        GRP groupMembers;
        conf.mtype = MSG_CONFIRM;
        int rcvmsg = 0;
        int i = 0;
        int foundid = 0;
        int numOfSent = 0;
        while(1){
            if(*end == 1) break;
            rcvmsg = msgrcv(clientServerCom, &buffer, (sizeof(int)*2 + sizeof(char)*352), 0, IPC_NOWAIT);
            if(rcvmsg == -1 && *end != 1) continue;
            if(*end == 1) exit(0);
            i = 0;
            foundid = 0;
            printf("@RECEIVED MESSAGE #%ld:\n", buffer.mtype);

            if(buffer.mtype == MSG_LOGOUT){
                userlist[buffer.senderID-1].logged = 0;
                conf.mtype = MSG_CONFIRM;
                conf.confirmation = 1;
                msgsnd(userlist[(buffer.senderID)-1].communication, &conf, sizeof(int), IPC_NOWAIT);
                printf("@RECEIVED LOG-OUT FROM #%d\n", buffer.senderID);
                userlist[buffer.senderID-1].communication = 0;
                continue;
            }
            if(buffer.mtype == MSG_USER_MESSAGE){ //Handling user message
                while(foundid == 0 && i<65){
                    if(!strcmp(userlist[i].usrNAME,buffer.receiverName)){
                        foundid = i+1;
                    }
                    ++i;
                }
                if(foundid == 0){
                    printf("@MESSAGE PASS FROM #%d TO USER #%d FAILED\n", buffer.senderID, foundid);
                    conf.confirmation = 0;
                    msgsnd(userlist[buffer.senderID-1].communication, &conf, sizeof(int), 0);
                    continue;
                } else {
                    if(userlist[foundid-1].logged == 0){
                        conf.confirmation = 0;
                        msgsnd(userlist[buffer.senderID-1].communication, &conf, sizeof(int), 0);
                        continue;
                    }
                    msg.mtyp = buffer.priority;
                    strcpy(msg.senderName, buffer.senderName);
                    strcpy(msg.groupName, buffer.groupName);
                    strcpy(msg.msg, buffer.message);
                    msgsnd(userlist[foundid-1].communication, &msg, sizeof(char)*320, 0);
                    conf.confirmation = 1;
                    msgsnd(userlist[buffer.senderID-1].communication, &conf, sizeof(int), 0);
                    printf("@PASSED MESSAGE FROM #%d TO USER #%d\n", buffer.senderID, foundid);
                    continue;
                }
                i = 0;
                foundid = 0;
                continue;
            }
            if(buffer.mtype == MSG_GROUP_MESSAGE){ //Handling group messages
                while(foundid == 0 && i<17){
                    if(!strcmp(grouplist[i].groupNAME, buffer.groupName)){
                        foundid = i+1;
                    }
                    ++i;
                }
                if(foundid == 0){
                    printf("@MESSAGE PASS FROM #%d TO GROUP #%d FAILED\n", buffer.senderID, foundid);
                    conf.confirmation = 0;
                    msgsnd(userlist[buffer.senderID-1].communication, &conf, sizeof(int), IPC_NOWAIT);
                    continue;
                } else {
                    msg.mtyp = buffer.priority;
                    strcpy(msg.senderName, buffer.senderName);
                    strcpy(msg.groupName, buffer.groupName);
                    strcpy(msg.msg, buffer.message);
                    numOfSent = 0;
                    for(int k = 0; k < 64; ++k){
                        if(grouplist[foundid-1].members[k] == 1){
                            if(userlist[k].logged == 1){
                                msgsnd(userlist[k].communication, &msg, sizeof(char)*320, IPC_NOWAIT);
                                numOfSent++;
                            }
                        }
                    }
                    if(numOfSent > 0){
                        conf.confirmation = 1;
                        msgsnd(userlist[buffer.senderID-1].communication, &conf, sizeof(int), IPC_NOWAIT);
                        printf("PASSED MESSAGE FROM #%d TO GROUP #%d\n", buffer.senderID, foundid);
                        i = 0;
                        foundid = 0;
                        continue;
                    } else {
                        printf("@MESSAGE PASS FROM #%d TO GROUP #%d FAILED\n", buffer.senderID, foundid);
                        conf.confirmation = 0;
                        msgsnd(userlist[buffer.senderID-1].communication, &conf, sizeof(int), IPC_NOWAIT);
                        i = 0;
                        foundid = 0;
                        continue;
                    }
                }
                continue;
            }
            if(buffer.mtype == MSG_ONLINE) { //Handling logged list
                loggedUsers.mtype = MSG_RECEIVE_ONLINE;
                for(int i = 0; i < 64; i++) loggedUsers.loggedusers[i] = 0; //Setting all values to 0
                for(int i = 0; i < 64; i++) if(userlist[i].logged == 1) loggedUsers.loggedusers[i] = 1;
                msgsnd(userlist[(buffer.senderID)-1].communication, &loggedUsers, sizeof(int)*64, 0);
                printf("@SENT ONLINE USERS LIST TO #%d\n",buffer.senderID);
                continue;
            }
            if(buffer.mtype == MSG_GROUP_MEMBERS) { //Handling group members list
                groupMembers.mtype = MSG_GROUP_MEMBERS;
                int grp;
                for(grp = 0; grp < 16; grp++){
                    if(!strcmp(buffer.groupName, grouplist[grp].groupNAME)) break;
                }
                if(grp == 16){
                    groupMembers.conf = 0;
                    msgsnd(userlist[(buffer.senderID)-1].communication, &groupMembers, sizeof(int)*65, 0);
                    continue;
                }
                groupMembers.conf = 1;
                for(int i = 0; i < 64; i++) groupMembers.groupmembers[i] = 0; //Set all values to 0
                for(int i = 0; i < 64; i++) if(grouplist[grp].members[i] == 1) groupMembers.groupmembers[i] = 1;
                msgsnd(userlist[(buffer.senderID)-1].communication, &groupMembers, sizeof(int)*65, 0);
                printf("@SENT GROUP MEMBER LIST TO #%d\n",buffer.senderID);
                continue;
            }
            if(buffer.mtype == MSG_GROUP_JOIN) { //Handling joining group
                conf.mtype = MSG_CONFIRM;
                foundid = atoi(buffer.groupName);
                grouplist[foundid-1].members[(buffer.senderID)-1] = 1;
                conf.confirmation = 1;
                msgsnd(userlist[(buffer.senderID)-1].communication, &conf, sizeof(int)*1, 0);
                printf("@ALLOWED USER #%d TO JOIN GROUP #%s\n",buffer.senderID, buffer.groupName);
                continue;
            }
            if(buffer.mtype == MSG_GROUP_LEAVE) { //Handling leaving group
                conf.mtype = MSG_CONFIRM;
                foundid = atoi(buffer.groupName);
                grouplist[foundid-1].members[(buffer.senderID)-1] = 0;
                conf.confirmation = 1;
                msgsnd(userlist[(buffer.senderID)-1].communication, &conf, sizeof(int)*1, 0);
                printf("@ALLOWED USER #%d TO LEAVE GROUP #%s\n",buffer.senderID, buffer.groupName);
                continue;
            }
        }
        exit(0);
    }

    if(fork()==0) { //Process waiting for !quit command
        char endcmd[8];
        while(1){
            scanf("%s", endcmd);
            if(*end == 1) {
                shmdt(end);
                exit(0);
            }
            if(!strcmp(endcmd,"!quit")){
                printf("Quitting...\n");
                quit(end);
                shmdt(end);
                break;
            }
            strcpy(endcmd,"");
        }
        exit(0);
    }

    while(1) {
        if(*end==1){
            shmdt(end);
            shmdt(userlist);
            shmdt(grouplist);
            printf("@SHUTDOWN IN 10 SECONDS\n");
            sleep(10); //Giving all children processes time to end 
            shmctl(quitserver, IPC_RMID, NULL);
            shmctl(SHMuserlist, IPC_RMID, NULL);
            shmctl(SHMgrouplist, IPC_RMID, NULL);
            msgctl(clientServerCom, IPC_RMID, NULL);

            break;
        } 
    }
    return 0;

}