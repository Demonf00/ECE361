//servr should check if the client is join two or more session
#include "conference.h"

Session sessions[MAX_MEETINGS];
ClientData database[MAX_USERS];
char buffer[BUFSIZ];
int numSessions = 0;
char* databasePath = "./clientData";
int sockfd;
int connfd[MAX_USERS];
int user_numbers = 0;
struct sockaddr_in servaddr, cliaddr;
struct message msg, response;
char server_message[5000], client_message[5000];

void encode(){
    bzero(server_message,sizeof(server_message));
    snprintf(server_message, sizeof(server_message),
    "%d;%d;%s;%s",msg.type,msg.size,msg.source,msg.data);
}
void decode(){
    bzero(response.data,MAX_DATA);
    // printf("1\n");
    char *item  = strtok(client_message, ";");
    char *contain[4];
    int i=0;
    while(item != NULL){
        contain[i++]=item;
        item  = strtok(NULL, ";");
    }
    // printf("2\n");
    response.type = atoi(contain[0]);
    response.size = atoi(contain[1]);
    // printf("3\n");
    // printf("%s\n", contain[2]);
    strcpy(response.source,contain[2]);
    // printf("?\n");
    strcpy(response.data,contain[3]);
    // printf("4\n");

    bzero(client_message,sizeof(client_message));

}
void text_recv(){
    printf("%s\n",response.data);
}

void init(void)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        database[i].status = 0;
        database[i].name[0] = '\0';
        database[i].password[0] = '\0';
        database[i].sessionList = NULL;
    }

    for (int i = 0; i < MAX_MEETINGS; ++i)
    {
        sessions[i].clientList = NULL;
        sessions[i].meetingName[0] = '\0';
        sessions[i].users = 0;
    }
    return;
}

void loadData(ClientData* database, char* sourcePath)
{
    FILE* fp = fopen(sourcePath, "r");
    if (fp == NULL)
    {
        perror("Unable to open the client database.\n");
        exit(1);
    }
    char chunk[128];
    int dataIndex = 0;
    while(fgets(chunk, 128, fp) != NULL)
    {
        int length = strlen(chunk);
        int split;
        for (int i = 0; i < length; ++i)
        {
            if (chunk[i] == ' ')
            {
                split = i;
                chunk[i] = '\0';
                break;
            }
        }
        // printf("%s", chunk);
        if (dataIndex < MAX_USERS)
        {
            strcpy(database[dataIndex].name, chunk);
            if (chunk[length - 1] == '\n')
                chunk[length - 1] = '\0';
            strcpy(database[dataIndex].password, &chunk[split+1]);
        }
        else
        {
            perror("Unable to open the client database.\n");
            exit(1);
        }
        // printf("name:%s, password:%s\n", database[dataIndex].name, database[dataIndex].password);
        // printf("%d\n", strcmp(database[dataIndex].name, database[dataIndex].password));
        dataIndex++;
    }
    fclose(fp);
}

bool checkLog(ClientData* database, unsigned char* name, unsigned char* password, int log, int fd)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (database[i].name[0] == '\0')
            return false;
        if (strcmp(database[i].name, name) == 0)
        {
            if (!log)
            {
                database[i].status = log;
                return true;
            }
            if (strcmp(database[i].password, password) == 0)
            {
                database[i].status = log;
                database[i].fd = fd;
                return true;
            }
        }    
    }
    return false;
}

int getSessionid(char* sessionName, Session* sessions)
{
    for (int i = 0; i < MAX_MEETINGS; ++i)
    {
        if (strcmp(sessions[i].meetingName, sessionName) == 0)
            return i;
    }
    return -1;// not found
}

int getClientid(ClientData* database, const ClientData* client)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (database[i].address == client->address && database[i].port == client->port)
            return i;
    }
    return -1;
}

bool joinSession(ClientData* database, int clientid, Session* session, int sessionid)
{
    if (session[sessionid].users == MAX_USERS_IN_A_MEETING)
        return false;
    if (session[sessionid].users == 0)
    {
        session[sessionid].clientList = (ClientId*)malloc(sizeof(ClientId));
        session[sessionid].clientList->next = NULL;
        session[sessionid].clientList->id = clientid;
        database[clientid].sessionList = (SessionId*)malloc(sizeof(SessionId));
        database[clientid].sessionList->id = sessionid;
        database[clientid].sessionList->next = NULL;
        session[sessionid].users++;
    }
    else{
        ClientId* current = session[sessionid].clientList;
        while(current->next != NULL)
        {
            // previous = current;
            current = current->next;
        }
        ClientId* newclient = (ClientId*)malloc(sizeof(ClientId));
        newclient->id = clientid;
        newclient->next = current->next;
        current->next = newclient;
        SessionId* newsession = (SessionId*)malloc(sizeof(SessionId));
        newsession->id = sessionid;
        newsession->next = NULL;
        database[clientid].sessionList = newsession;
        session[sessionid].users++;
    }
    return true;
}


bool quitSession(ClientData* database, int clientid, Session* session, int sessionid)
{
    if (session[sessionid].users == 0)
        return false;
    if (session[sessionid].users == 1)
    {
        session[sessionid].users--;
        free(session[sessionid].clientList);
        session[sessionid].clientList = NULL;
        free(database[clientid].sessionList);
        database[clientid].sessionList = NULL;
    }

}

void clear(void)
{
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Missing or too many arguments!\nThe correct usage should be: server port\n");
	    exit(1);
    }
    int port = atoi(argv[1]);

    //need to check if the port is possible here.
    init();
    loadData(database, databasePath);  
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "Socket creation failed!\n");
	    exit(1);
    }
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    if ((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    int len = sizeof(cliaddr);

    while(true)
    {
        if ((listen(sockfd, 5)) != 0) {
            printf("Listen failed...\n");
            // continue;
        }
        else
            printf("Server listening..\n");
        connfd[user_numbers] = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
        if (connfd[user_numbers] < 0) {
            printf("server accept failed...\n");
            // continue;
        }
        else{
            printf("server accept the client...\n");
            user_numbers++;
        }
        for (int i = 0; i < user_numbers; ++i)
        {
            // printf("hello %d\n", i);
            if(read(connfd[i], client_message, sizeof(client_message))<=0)
            {
                printf("read socket not success\n");
                continue;
            }
            // printf("receive messgae from client!\n");
            decode();
            // printf("receive messgae from client!\n");
            if(response.type == LOGIN)
            {
                printf("login\n");
                if (checkLog(database, response.source, response.data, 1, connfd[i]))
                {
                    msg.type = LO_ACK;
                    msg.size = 0;
                    msg.data[0] = '\0';
                    msg.source[0] = '\0';
                }
                else{
                    msg.type = LO_NAK;
                    msg.size = 0;
                    msg.data[0] = '\0';
                    msg.source[0] = '\0';
                }
                encode();
                while (write(connfd[i], server_message, sizeof(server_message)) <= 0)
                    printf("Server: sent back log failed\n");
                printf("Ack sent\n");
            }
            else if(response.type == EXIT)
            {
                if (!checkLog(database, response.source, response.data, 0, connfd[i]))
                    printf("Server: client %s exit error\n", response.source);
            }
            else if(response.type == JOIN)
            {

            }
            else if(response.type == LEAVE_SESS)
            {

            }
            else if(response.type == NEW_SESS)
            {

            }
            else if(response.type == MESSAGE)
            {

            }
            else if(response.type == QUERY)
            {

            }
            else
            {

            }
        }

    }
    

    return 0;
}