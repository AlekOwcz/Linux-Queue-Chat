#include<stdio.h>

void help(){
    printf("\n!help - Lists aviable commands.\n"
    "!login - Logs you in.\n"
    "!logout - Logs you out.\n"
    "!online - List currently logged-in users\n"
    "!members - List members of a specified group\n"
    "!usermessage - Message a specified user\n"
    "!groupmessage - Message members of a specified group\n"
    "!groups - List aviable groups\n"
    "!groupjoin - Join specified group\n"
    "!groupleave - Leave specified group\n"
    "!quit - logs you out and quits the program\n\n");
    return;
}