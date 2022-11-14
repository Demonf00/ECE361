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
#define MIN(i, j) (((i) < (j)) ? (i) : (j))
#define MAX_NAME 100
#define MAX_DATA 2000

typedef enum {
    LOGIN,//Login with the server
    LO_ACK,//Acknowledge successful login
    LO_NAK,//Negative acknowledgement of login
    EXIT,//Exit from the server
    JOIN,//Join a conference session
    JN_ACK,//Acknowledge successful conference session join
    JN_NAK,//Negative acknowledgement of joining the session
    LEAVE_SESE,//Leave a conference session
    NEW_SESS,//Create new conference session
    NS_ACK,//Acknowledge new conference session
    MESSAGE,//Send a message to the session or display the message if it is received
    QUERY,//Get a list of online users and available sessions
    QU_ACK//Reply followed by a list of users online
} Control;

struct message {
    Control type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};