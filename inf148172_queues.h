#define MSG_USER_MESSAGE 4
#define MSG_LOGIN 5
#define MSG_LOGOUT 6
#define MSG_USERS_LOGGED 7
#define MSG_GROUP_MEMBERS 8
#define MSG_GROUP_JOIN 9
#define MSG_GROUP_LEAVE 10
#define MSG_GROUP_MESSAGE 11
#define MSG_CONFIRM 12
#define MSG_RECEIVE 4
#define MSG_ONLINE 13
#define MSG_RECEIVE_ONLINE 14


typedef struct ServerToClient {
    long mtype;
    int ID;
    int queue;
} STC;

typedef struct ConfirmationQueue {
    long mtype;
    int confirmation;
} Confirm;

typedef struct ClientServerCom {
    long mtype;
    int senderID;
    char senderName[32];
    char receiverName[32];
    char groupName[32];
    char message[256];
    int priority;
} CSC;

typedef struct Message {
    long mtyp;
    char senderName[32];
    char groupName[32];
    char msg[256];
} MSG;

typedef struct Logged {
    long mtype;
    int loggedusers[64];
} LGD;

typedef struct GroupMembers {
    long mtype;
    int groupmembers[64];
    int conf;
} GRP;
