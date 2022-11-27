//servr should check if the client is join two or more session
#include "conference.h"

Session sessions[MAX_MEETINGS];
ClientData database[MAX_USERS];
char buffer[BUFSIZ];
int numSessions = 0;

int sockfd;
int connfd;
int user_numbers = 0;
struct sockaddr_in servaddr, cliaddr;
struct message msg, response;
pid_t childpid;
char server_message[5000], client_message[5000];

int main(void)
{
    char databasePath[] = "./clientTest";
    strcpy(response.source, "c");
    strcpy(response.data, "dddd");
    printf("%s\n", databasePath);
    FILE* fp = fopen(databasePath, "a");
    if (fp == NULL)
    {
        perror("Unable to open the client database.\n");
        exit(1);
    }
    char chunk[128];
    sprintf(chunk, "\n%s %s", response.source, response.data);
    printf("%s\n", chunk);
    fputs(chunk, fp);
    fclose(fp);
    return 0;
}