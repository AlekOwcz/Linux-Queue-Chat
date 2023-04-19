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

void groups(){
    int fd = open("groups.txt",O_RDONLY);
    char c = 'a';
    int r;
    int idx;
    char groupName[32];
    while(c != '\n') read(fd, &c, 1); //skipping first line
    while(1){
        strcpy(groupName, "");
        while(c != '\t') { //reading ID
            r = read(fd, &c, 1);
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
        idx = 0;
        c = 'a';
        do { //reading group name from file
            read(fd, &groupName[idx++], 1);
        } while(groupName[idx-1] != '\t');
        groupName[idx-1] = '\0'; //removing last \t character
        printf("%s\t",groupName);
        idx = 0;
        while(c != '\n') read(fd, &c, 1);
        c = 'a';    
    }
    close(fd);
    return;
}
