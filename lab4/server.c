//servr should check if the client is join two or more session
#include "conference.h"

Session* sessions;
ClientData* database;
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
    if (database[clientid].sessionList != NULL && database[clientid].sessionList->id == sessionid)
        return false;
    database[clientid].sessionList = (SessionId*)malloc(sizeof(SessionId));
    database[clientid].sessionList->id = sessionid;
    session[sessionid].users++;
    // if (session[sessionid].users == 0)
    // {
    //     session[sessionid].clientList = (ClientId*)malloc(sizeof(ClientId));
    //     session[sessionid].clientList->next = NULL;
    //     session[sessionid].clientList->id = clientid;
    //     database[clientid].sessionList = (SessionId*)malloc(sizeof(SessionId));
    //     database[clientid].sessionList->id = sessionid;
    //     database[clientid].sessionList->next = NULL;
    //     session[sessionid].users++;
    // }
    // else{
    //     ClientId* newclient = (ClientId*)malloc(sizeof(ClientId));
    //     newclient->id = clientid;
    //     printf("%d %d\n", clientid, session[sessionid].clientList->id);
    //     newclient->next = session[sessionid].clientList;
    //     session[sessionid].clientList = newclient;
    //     printf("%d %d\n", session[sessionid].clientList->id,session[sessionid].clientList->next->id );
    //     SessionId* newsession = (SessionId*)malloc(sizeof(SessionId));
    //     newsession->id = sessionid;
    //     newsession->next = NULL;
    //     database[clientid].sessionList = newsession;
    //     session[sessionid].users++;
    // }
    return true;
}


bool quitSession(ClientData* database, int clientid, Session* session, int sessionid)
{
    if (session[sessionid].users == 0)
        return false;
    free(database[clientid].sessionList);
    database[clientid].sessionList = NULL;
    session[sessionid].users--;
    if (session[sessionid].users == 0)
    {
        session[sessionid].meetingName[0] = '\0';
    }
    // if (session[sessionid].users == 1)
    // {
    //     session[sessionid].users--;
    //     free(session[sessionid].clientList);
    //     session[sessionid].clientList = NULL;
    //     free(database[clientid].sessionList);
    //     database[clientid].sessionList = NULL;
    //     session[sessionid].meetingName[0] = '\0';
    // }
    // else{
    //     ClientId* current = session[sessionid].clientList;
    //     ClientId* previous = NULL;
    //     while(current != NULL && current->id != clientid)
    //     {
    //         previous = current;
    //         current = current->next;
    //     }
    //     if (current == NULL)
    //         return false;
    //     else if (current == session[sessionid].clientList)
    //     {
    //         session[sessionid].clientList = current->next;
    //         free(current);
    //         session[sessionid].users--;
    //         free(database[clientid].sessionList);
    //         database[clientid].sessionList = NULL;
    //     }
    //     else{
    //         previous->next = current->next;
    //         free(current);
    //         session[sessionid].users--;
    //         free(database[clientid].sessionList);
    //         database[clientid].sessionList = NULL;
    //     }
    // }
    return true;
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

    sessions = mmap(NULL, sizeof(Session)*MAX_MEETINGS, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    database = mmap(NULL, sizeof(ClientData)*MAX_USERS, PROT_READ | PROT_WRITE, 
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
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
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    while(true)
    {
        connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len);
        // printf("%d\n", connfd[user_numbers]);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else{

            printf("Server accepted the client from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            user_numbers++;
        }
        
        if ((childpid = fork()) == 0)
        {
            close(sockfd);
            while(true)
            {
                // printf("Server: waiting for client %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                memset(&client_message, 0, sizeof(client_message));
                if(read(connfd, client_message, sizeof(client_message))<=0)
                {
                    printf("read socket not success\n");
                    continue;
                }
                // printf("receive messgae from client!\n");
                decode();
                // printf("receive messgae from client!\n");
                if(response.type == LOGIN)
                {
                    // printf("login\n");
                    if (checkLog(database, response.source, response.data, 1, connfd))
                    {
                        printf("login\n");
                        msg.type = LO_ACK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    else{
                        printf("login failed\n");
                        msg.type = LO_NAK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back log failed\n");
                    printf("Ack sent\n");
                }
                else if(response.type == EXIT)
                {
                    if (!checkLog(database, response.source, response.data, 0, connfd))
                    {
                        printf("Server: client %s exit error\n", response.source);
                        break;
                    }
                    printf("Server disconnected the client from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                    break;
                }
                else if(response.type == JOIN)
                {
                    int clientid = getClientid(database, response.source);
                    int sessionid = getSessionid(sessions, response.data);
                    printf("client %s:%d, session %s:%d\n",response.source, clientid, response.data, sessionid);
                    assert(clientid != -1);
                    if (sessionid != -1 && joinSession(database, clientid, sessions, sessionid))
                    {
                        printf("Client %s joined %s \n", response.source, response.data);
                        msg.type = JN_ACK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    else{
                        printf("Client %s failed to join %s \n", response.source, response.data);
                        msg.type = JN_NAK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back join failed\n");
                    printf("Ack sent\n");
                }
                else if(response.type == LEAVE_SESS)
                {
                    int clientid = getClientid(database, response.source);
                    int sessionid = database[clientid].sessionList->id;
                    assert(clientid != -1);
                    if (sessionid != -1 && quitSession(database, clientid, sessions, sessionid))
                    {
                        printf("Client %s quited %s \n", response.source, sessions[sessionid].meetingName);
                        msg.type = LEAVE_SESS;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    else
                    {
                        printf("Client %s failed to quit %s \n", response.source, sessions[sessionid].meetingName);
                        msg.type = ERROR;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back quit failed\n");
                    printf("Ack sent\n");
                }
                else if(response.type == NEW_SESS)
                {
                    for (int i = 0; i < MAX_MEETINGS; ++i)
                    {
                        if(sessions[i].meetingName[0] == '\0')
                        {
                            strcpy(sessions[i].meetingName, response.data);
                            break;
                        }
                    }
                    printf("Sessions:\n");
                    for (int i = 0; i < MAX_MEETINGS; ++i)
                    {
                        if (sessions[i].meetingName[0] != '\0')
                            printf("%s\n", sessions[i].meetingName);
                    }
                    msg.type = NS_ACK;
                    msg.size = 0;
                    msg.data[0] = ' ';
                    msg.source[0] = ' ';
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back new session failed\n");
                    printf("Server: session %s created\n", response.data);
                }
                else if(response.type == MESSAGE)
                {
                    // memcpy(server_message,client_message,sizeof(client_message));
                    int clientid = getClientid(database, response.source);
                    
                    memcpy((struct message*)&msg, (struct message*)&response, sizeof(struct message));
                    printf("Server: client %s, %d wanna sent message\n", response.source, clientid);
                    if (database[clientid].sessionList == NULL)
                    {
                        printf("Server: client %s not in a session\n", response.source);
                        msg.type = ERROR;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                        encode();
                        while (write(connfd, server_message, sizeof(server_message)) <= 0)
                            printf("Server: sent back message failed\n");
                        continue;
                    }
                    // int sessionid = database[clientid].sessionList->id;
                    // ClientId* current = sessions[sessionid].clientList;
                    // while(current != NULL)
                    // {
                    //     printf("%d %s\n", current->id, database[current->id].name);
                    //     while (write(database[current->id].fd,server_message, sizeof(server_message))<=0)
                    //         printf("Server: Sent message to %s failed\n", database[current->id].name);
                    //     if (current->next != current)
                    //         current = current->next;
                    //     else break;
                    // }
                    int sessionid = database[clientid].sessionList->id;
                    encode();
                    printf("Server: sent %s\n", server_message);
                    for (int i = 0; i < MAX_USERS; ++i)
                    {
                        if (database[i].sessionList != NULL && database[i].sessionList->id == sessionid)
                        {
                            printf("Server: sent message to %s:%d\n", database[i].name, i);
                            while(write(database[i].fd, server_message, sizeof(server_message))<=0);
                                printf("Server: Sent message to %s failed\n", database[i].name);
                        }
                    }
                }
                else if(response.type == QUERY)
                {
                    bzero(buffer, BUFSIZ);
                    strcpy(buffer, "Client:\n");
                    
                    for (int i = 0; i < MAX_USERS; ++i)
                    {
                        if (database[i].status == 1)
                        {   
                            // printf("%s\n", database[i].name);
                            strcat(buffer, database[i].name);
                            strcat(buffer, "\n");
                        }
                        
                    }
                    strcat(buffer, "Sessions:\n");
                    for (int i = 0; i < MAX_MEETINGS; ++i)
                    {
                        if (sessions[i].meetingName[0] != '\0')
                        {
                            // printf("%s\n", sessions[i].meetingName);
                            strcat(buffer, sessions[i].meetingName);
                            strcat(buffer, "\n");
                        }
                    }
                    // printf("%s\n", buffer);
                    msg.type = QU_ACK;
                    msg.size = strlen(buffer);
                    msg.source[0] = ' ';
                    memcpy(msg.data, buffer,msg.size);
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back query failed\n");
                }
                else
                {
                    msg.type = ERROR;
                    msg.size = 0;
                    msg.data[0] = ' ';
                    msg.source[0] = ' ';
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back error failed\n");
                    printf("Error!\n");
                }
            }
        }

    }
    

    return 0;
}