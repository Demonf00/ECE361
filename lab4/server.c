//servr should check if the client is join two or more session
#include "conference.h"

Session sessions[MAX_MEETINGS];
ClientData database[MAX_USERS];
char buffer[BUFSIZ];
char* databasePath = "./clientData";

void init(void)
{
    for (int i = 0; i < MAX_USERS; ++i)
    {
        database[i].status = 0;
        database[i].name[0] = '\0';
        database[i].password[0] = '\0';
    }

    for (int i = 0; i < MAX_MEETINGS; ++i)
    {
        sessions[i].clientlist = NULL;
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