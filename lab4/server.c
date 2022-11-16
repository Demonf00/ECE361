//servr should check if the client is join two or more session
#include "conference.h"

Session sessions[MAX_MEETINGS];
ClientData database[MAX_USERS];
char buffer[BUFSIZ];
int numSessions = 0;
char* databasePath = "./clientData";

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
        printf("name:%s, password:%s\n", database[dataIndex].name, database[dataIndex].password);
        // printf("%d\n", strcmp(database[dataIndex].name, database[dataIndex].password));
        dataIndex++;
    }
    fclose(fp);
}

bool checkLog(ClientData* database, char* name, char* password, int log)
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
    // if (session.users == MAX_USERS_IN_A_MEETING)
    //     return false;
//     if (session.users == 0)
//     {
//         session.clientlist = client;
//         client->next = NULL;
//         session.users++;
//     }
//     else{
//         Client* current = session.clientlist;
//         // Client* previous = NULL;
//         while(current->next != NULL)
//         {
//             // previous = current;
//             current = current->next;
//         }
//         client->next = current->next;
//         current->next = client;
//         session.users++;
//     }
// }
    return true;
}

bool quitSession(ClientData* database, int clientid, Session* session, int sessionid)
{
    // Client* current = session.clientlist;
    // Client* previous = NULL;
    // while(current != NULL && )
    // {

    // }
    // if (current == NULL)
    return false;

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
    init();
    loadData(database, databasePath);
    

    return 0;
}