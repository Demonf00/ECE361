//servr should check if the client is join two or more session
#include "conference.h"

Session sessions[MAX_MEETINGS];
ClientData database[MAX_USERS];
char buffer[BUFSIZ];
int numSessions = 0;
char* databasePath = "./clientData";
int sockfd;
int connfd;
int user_numbers = 0;
struct sockaddr_in servaddr, cliaddr;
struct message msg, response;
pid_t childpid;
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
    // database = (ClientData*)malloc(sizeof(ClientData)*MAX_USERS);
    // sessions = (Session*)malloc(sizeof(Session)*MAX_MEETINGS);
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
            if (database[i].status == log)
                return false;
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

int getSessionid(Session* sessions, unsigned char* name)
{
    for (int i = 0; i < MAX_MEETINGS; ++i)
    {
        if (strcmp(sessions[i].meetingName, name) == 0)
            return i;
    }
    return -1;// not found
}

int getClientid(ClientData* database, unsigned char* name)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        if (strcmp(database[i].name, name) == 0)
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
        // ClientId* current = session[sessionid].clientList;
        // while(current->next != NULL)
        // {
        //     // previous = current;
        //     current = current->next;
        //     printf("%d %d", current->id, session[sessionid].clientList->id);
        // }
        // printf("%d %d", current->id, session[sessionid].clientList->id);
        ClientId* newclient = (ClientId*)malloc(sizeof(ClientId));
        newclient->id = clientid;
        // printf("%d %d\n", clientid, session[sessionid].clientList->id);
        newclient->next = session[sessionid].clientList;
        session[sessionid].clientList = newclient;
        // current->next = newclient;
        // printf("%d %d\n", session[sessionid].clientList->id,session[sessionid].clientList->next->id );
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
    else{
        ClientId* current = session[sessionid].clientList;
        ClientId* previous = NULL;
        while(current != NULL && current->id != clientid)
        {
            previous = current;
            current = current->next;
        }
        if (current == NULL)
            return false;
        else if (current == session[sessionid].clientList)
        {
            session[sessionid].clientList = current->next;
            free(current);
            session[sessionid].users--;
            free(database[clientid].sessionList);
            database[clientid].sessionList = NULL;
        }
        else{
            previous->next = current->next;
            free(current);
            session[sessionid].users--;
            free(database[clientid].sessionList);
            database[clientid].sessionList = NULL;
        }
    }
    return true;
}

void clear(void)
{
    return;
}

int main(int argc, char *argv[])
{

    // sessions = mmap(NULL, sizeof(Session)*MAX_MEETINGS, PROT_READ | PROT_WRITE, 
    //                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // database = mmap(NULL, sizeof(ClientData)*MAX_USERS, PROT_READ | PROT_WRITE, 
    //                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    joinSession(database, 0, sessions, 0);
    ClientId* current = sessions[0].clientList;
    while(current != NULL)
    {
        printf("%d\n", current->id);
        current = current->next;
    }
    printf("\n");
    joinSession(database, 1, sessions, 0);
    current = sessions[0].clientList;
    while(current != NULL)
    {
        printf("%d\n", current->id);
        current = current->next;
    }
    printf("\n");
    joinSession(database, 2, sessions, 0);
    current = sessions[0].clientList;
    while(current != NULL)
    {
        printf("%d\n", current->id);
        current = current->next;
    }
    printf("\n");
    quitSession(database, 0, sessions, 0);
    current = sessions[0].clientList;
    while(current != NULL)
    {
        printf("%d\n", current->id);
        current = current->next;
    }
    printf("\n");
    quitSession(database, 2, sessions, 0);
    current = sessions[0].clientList;
    while(current != NULL)
    {
        printf("%d\n", current->id);
        current = current->next;
    }
    printf("\n");
    quitSession(database, 1, sessions, 0);
    current = sessions[0].clientList;
    while(current != NULL)
    {
        printf("%d\n", current->id);
        current = current->next;
    }
    printf("\n");

    return 0;
}