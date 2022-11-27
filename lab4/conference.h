#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <sys/mman.h>
#include <strings.h> // bzero()
#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX_CMD 80
#define MAX_USER_NAME 100
#define MAX_USER_PASSWORD 100
#define MAX_MEETING_NAME 100
#define MAX_DATA 2000
#define MAX_USERS 100
#define MAX_USERS_IN_A_MEETING 20
#define MAX_MEETINGS 100
#define MAX_PASSWORD_LENGTH 100


typedef enum {
    REG,//register
    REG_ACK,//ack for reg
    REG_NAK,//nak for reg
    LOGIN,//Login with the server
    LO_ACK,//Acknowledge successful login
    LO_NAK,//Negative acknowledgement of login
    EXIT,//Exit from the server
    JOIN,//Join a conference session
    JN_ACK,//Acknowledge successful conference session join
    JN_NAK,//Negative acknowledgement of joining the session
    LEAVE_SESS,//Leave a conference session
    NEW_SESS,//Create new conference session
    NS_ASK,//ask if need password
    NS_PWD,//password
    NS_NO,//no need password
    NS_YES,//need passwords
    NS_ASK_ACK,//ack for yes and no
    NS_PWD_ACK,//ack for password
    NS_ACK,//Acknowledge new conference session
    NS_NAK,//error when create new conf
    MESSAGE,//Send a message to the session or display the message if it is received
    QUERY,//Get a list of online users and available sessions
    QU_ACK,//Reply followed by a list of users online
    QUIT,
    ERROR
} Control;

struct message {
    Control type;
    unsigned int size;
    unsigned char source[MAX_USER_NAME];
    unsigned char data[MAX_DATA];
};

typedef struct sessionId{
    int id;
    struct sessionId* next;
}SessionId;

typedef struct clientId{
    int id;
    struct clientId* next;
}ClientId;

typedef struct session{
    unsigned char meetingName[MAX_MEETING_NAME];
    int users;
    unsigned char password[MAX_PASSWORD_LENGTH];
    ClientId* clientList;
}Session;

typedef struct clientData{
    unsigned char name[MAX_USER_NAME];
    unsigned char password[MAX_USER_PASSWORD];
    in_addr_t address;
    int port;
    int fd;
    int status;//0 for not log in, 1 for log in
    SessionId* sessionList;
}ClientData;


void init(void); //init everything
void loadData(ClientData* database, char* sourcePath);//load data from the source database
bool checkLog(ClientData* database, unsigned char* name, unsigned char* password, int log, int fd);//if log == 1, log in, if log == 0, quit.
int getSessionid(Session* sessions, unsigned char* name);//get session id
int getClientid(ClientData* database, unsigned char* name);//get client id
//need check if already in one session when join.
bool joinSession(ClientData* database, int clientid, Session* session, int sessionid);//join in a session, if not exist, create one, if full, return false.
bool quitSession(ClientData* database, int clientid, Session* session, int sessionid);//quit from a session, if no one in it, delete it.
void clear(void); //clear everything
