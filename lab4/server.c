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
int tempSessionId = -1;
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
        database[i].sesid = -1;
    }

    for (int i = 0; i < MAX_MEETINGS; ++i)
    {
        sessions[i].clientList = NULL;
        sessions[i].meetingName[0] = '\0';
        sessions[i].password[0] = '\0';
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
    if (database[clientid].sesid != -1)
        return false;
    database[clientid].sesid = sessionid;
    session[sessionid].users++;
    return true;
}


bool quitSession(ClientData* database, int clientid, Session* session, int sessionid)
{
    if (session[sessionid].users == 0)
        return false;
    database[clientid].sesid = -1;
    session[sessionid].users--;
    if (session[sessionid].users == 0)
    {
        session[sessionid].meetingName[0] = '\0';
    }
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
    struct timeval timeout;
        
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
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
            // setsockopt (connfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof timeout);
            int count = 0;
            while(true)
            {
                // printf("Server: waiting for client %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                memset(&client_message, 0, sizeof(client_message));
                if(read(connfd, client_message, sizeof(client_message))<=0)
                {
                    // printf("read socket not success\n");
                    count++;
                    if (count >= 20)
                    {
                        for (int i = 0; i < MAX_USERS; ++i)
                        {
                            if (database[i].fd == connfd)
                            {
                                database[i].status = 0;//log out
                                quitSession(database, i, sessions, database[i].sesid);
                                printf("Server: Disconnected client %s:%d\n", database[i].name, i);
                            }
                        }
                            
                        close(connfd);
                        exit(0);
                    }
                    continue;
                }
                count = 0;
                // printf("receive messgae from client!\n");
                decode();
                // printf("receive messgae from client!\n");
                printf("Current stat:\n");
                for (int i = 0; i < MAX_USERS; ++i)
                    if (database[i].sesid != -1)
                        printf("Id: %d, name: %s, session: %d\n", i, database[i].name, database[i].sesid);
                if (response.type == REG)
                {
                    int full = 0;
                    for (int i = 0; i < MAX_USERS; ++i)
                    {
                        if (strcmp(database[i].name, response.source) == 0)
                        {
                            msg.type = REG_NAK;
                            msg.size = 0;
                            msg.data[0] = ' ';
                            msg.source[0] = ' ';
                            full = 1;
                            break;
                        }
                        if (database[i].name[0] == '\0')
                        {
                            msg.type = REG_ACK;
                            msg.size = 0;
                            msg.data[0] = ' ';
                            msg.source[0] = ' ';
                            strcpy(database[i].name, response.source);
                            strcpy(database[i].password, response.data);
                            FILE* fp = fopen(databasePath, "a");
                            if (fp == NULL)
                            {
                                perror("Unable to open the client database.\n");
                                exit(1);
                            }
                            char chunk[128];
                            sprintf(chunk, "\n%s %s", response.source, response.data);
                            fputs(chunk, fp);
                            fclose(fp);
                            full = 1;
                            break;
                        }
                    }
                    if (full == 0)
                    {
                        msg.type = REG_NAK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back reg failed\n");
                    printf("Ack sent\n");
                }
                else if(response.type == LOGIN)
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
                    if (database[clientid].sesid != -1)
                        goto already_join;
                    
                    if (sessionid == -1){
                        already_join:                     
                        msg.type = JN_NAK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                        printf("Client %s failed to join %s \n", response.source, response.data);
                        // sprintf(msg)
                    }
                    else if(sessions[sessionid].password[0] == '\0')
                    {
                        if (!joinSession(database, clientid, sessions, sessionid))
                            goto already_join;
                        printf("Client %s joined %s \n", response.source, response.data);
                        msg.type = JN_ACK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                    }
                    else{
                        msg.type = JN_ASK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                        tempSessionId = sessionid;
                        printf("Ask client %s for password to join session %s\n", response.source, response.data);
                    }

                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back join failed\n");
                    printf("Ack sent\n");
                }
                else if(response.type == JN_PWD)
                {
                    if (tempSessionId == -1) goto join_error;
                    int clientid = getClientid(database, response.source);
                    if (strcmp(sessions[tempSessionId].password, response.data) == 0)
                    {
                        msg.type = JN_ACK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                        printf("Client %s joined %s \n", response.source, sessions[tempSessionId].meetingName);
                        if (!joinSession(database, clientid, sessions, tempSessionId)) goto join_error;
                        tempSessionId == -1;
                    }
                    else
                    {
                        join_error:
                        msg.type = JN_NAK;
                        msg.size = 0;
                        msg.data[0] = ' ';
                        msg.source[0] = ' ';
                        printf("Client %s failed to join the meeting \n", response.source); 
                    }
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back join failed\n");
                    printf("Ack sent\n");
                }
                else if(response.type == LEAVE_SESS)
                {
                    int clientid = getClientid(database, response.source);
                    int sessionid = database[clientid].sesid;
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
                        if (strcmp(sessions[i].meetingName, response.data) == 0)
                        {
                            msg.type = NS_NAK;
                            msg.size = 0;
                            msg.data[0] = ' ';
                            msg.source[0] = ' ';
                            printf("Session %s create failed due to exist the same conference.\n", response.data);
                            goto back;
                        }
                    }
                    for (int i = 0; i < MAX_MEETINGS; ++i)
                    {
                        if(sessions[i].meetingName[0] == '\0')
                        {
                            tempSessionId = i;
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
                    
                    msg.type = NS_ASK;
                    msg.size = 0;
                    msg.data[0] = ' ';
                    msg.source[0] = ' ';
                    back:
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back new session failed\n");
                    printf("Server: session %s created\n", response.data);
                }
                else if(response.type == NS_PWD)
                {
                    msg.type = NS_ACK;
                    if (tempSessionId == -1)
                        msg.type = NS_NAK;
                    strcpy(sessions[tempSessionId].password, response.data);
                    tempSessionId = -1;
                    msg.size = 0;
                    msg.data[0] = ' ';
                    msg.source[0] = ' ';
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back new session failed\n");
                    printf("Server: session %s password set\n", sessions[tempSessionId].meetingName);
                }
                else if(response.type == NS_NO)
                {
                    msg.type = NS_ACK;
                    if (tempSessionId == -1)
                        msg.type = NS_NAK;
                    msg.size = 0;
                    msg.data[0] = ' ';
                    msg.source[0] = ' ';
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back new session failed\n");
                    printf("Server: session %s password set\n", sessions[tempSessionId].meetingName);
                }
                else if(response.type == MESSAGE)
                {
                    int clientid = getClientid(database, response.source);
                    memcpy((struct message*)&msg, (struct message*)&response, sizeof(struct message));
                    printf("Server: client %s, %d wanna sent message from session %s\n", response.source, clientid, sessions[database[clientid].sesid].meetingName);
                    if (database[clientid].sesid == -1)
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
                    int sessionid = database[clientid].sesid;
                    encode();
                    printf("Server: sent %s to session id %d\n", server_message, sessionid);
                    for (int i = 0; i < MAX_USERS; ++i)
                    {
                        if (database[i].sesid == sessionid)
                        {
                            printf("Server: sent message to %s:%d\n", database[i].name, database[i].sesid);
                            int fail = 0;
                            int ew;
                            while((ew = write(database[i].fd, server_message, sizeof(server_message)))<=0 && fail <20) fail++;
                            if (fail < 20) printf("Server: Sent message to %s success!\n", database[i].name);
                            else printf("Server: Sent message to %s failed!\n", database[i].name);
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
                // else if(response.type == QUIT){
                    
                // }
                else
                {
                    msg.type = ERROR;
                    msg.size = 0;
                    msg.data[0] = ' ';
                    msg.source[0] = ' ';
                    encode();
                    while (write(connfd, server_message, sizeof(server_message)) <= 0)
                        printf("Server: sent back error failed\n");
                    printf("Error! Unknow flag\n");
                }
            }
        }

    }
    

    return 0;
}