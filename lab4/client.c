#include "conference.h"

// https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
// https://aticleworld.com/socket-programming-in-c-using-tcpip/


int sock_desc, connfd;
struct sockaddr_in servaddr;
struct message msg, response;
char server_message[5000], client_message[5000];

void encode(){
    bzero(client_message,sizeof(client_message));
    snprintf(client_message, sizeof(client_message),
    "%d;%d;%s;%s",msg.type,msg.size,msg.source,msg.data);
}
void decode(){
    bzero(response.data,MAX_DATA);
    char *item  = strtok(server_message, ";");
    char *contain[4];
    int i=0;
    while(item != NULL){
        contain[i++]=item;
        item  = strtok(NULL, ";");
    }
    response.type = atoi(contain[0]);
    response.size = atoi(contain[1]);
    strcpy(response.source,contain[2]);
    strcpy(response.data,contain[3]);

    bzero(server_message,sizeof(server_message));

}

void text_recv(){
    printf("%s\n",response.data);
}

int main()
{

    while (true)
    {
        char input[2000], cmd[2000];
        fgets(input, 2000, stdin);

        strcpy(cmd,input);
        char *command = strtok(cmd, " \n");

        if (strcmp(command, "/login") == 0)
        {
            // /login <client ID> <password> <server-IP> <server-port>
            printf("in login mode\n");

            char *info[4];
            int i=0;
            command  = strtok(NULL, " ");
            while(command != NULL&&i<4){
                printf("%d\n",i);
                info[i++]=command;
                command  = strtok(NULL, " ");
            }
            if(i<3||command != NULL){
                printf("number of agument incorrect\n");
                continue;
            }
            char *name = info[0];
            char *password=info[1]; 

            // using default protocol->TCP
            sock_desc = socket(AF_INET, SOCK_STREAM, 0);
            if (sock_desc == -1){
                printf("socket creation failed\n");
                return -1;
            }

            // clear server socket info
            bzero(&servaddr, sizeof(servaddr));

            // assign IP, PORT
            servaddr.sin_family = AF_INET;
            servaddr.sin_addr.s_addr = inet_addr(info[2]);
            servaddr.sin_port = htons(atoi(info[3]));

            // connect the client socket to server socket
            if (connect(sock_desc, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
            {
                printf("connection with the server failed\n");
                return -1;
            }

            
            msg.type = LOGIN;
            msg.size = strlen(info[1]);
            strcpy(msg.source,info[0]);
            strcpy(msg.data,info[1]);

            encode();
            if(write(sock_desc, client_message, sizeof(client_message))<=0){
                printf("socket sent not success");
                continue;
            }
            
            if (read(sock_desc, server_message, sizeof(server_message))<=0){
                printf("read socket not success");
                continue;
            }
            decode();
            while(response.type==MESSAGE){
                text_recv();
                if (read(sock_desc, server_message, sizeof(server_message))<=0){
                    printf("read socket not success");
                    break;
                }
                decode();
            }
            if(response.type!=LO_ACK){
                printf("login not success\n");
                if(response.type==LO_NAK)
                    printf("%s\n", response.data);
            }

        }
        else if (strcmp(command, "/logout") == 0){
            //reset client
            printf("in logout mode\n");

            bzero(msg.data,MAX_DATA);
            msg.type = EXIT;
            msg.size = 0;

            encode();
            if(write(sock_desc, client_message, sizeof(client_message))<=0){
                printf("socket sent not success");
                continue;
            }
            
            close(sock_desc);
        }
        else if (strcmp(command, "/joinsession>") == 0){
            // /joinsession <session ID>
            printf("in joinsession mode\n");
            command  = strtok(NULL, " ");
            char *session_id = command;

            bzero(msg.data,MAX_DATA);
            msg.type = JOIN;
            msg.size = strlen(command);
            strcpy(msg.data,command);

            encode();
            if(write(sock_desc, client_message, sizeof(client_message))<=0){
                printf("socket sent not success");
                continue;
            }
            
            if (read(sock_desc, server_message, sizeof(server_message))<=0){
                printf("read socket not success");
                continue;
            }
            decode();
            while(response.type==MESSAGE){
                text_recv();
                if (read(sock_desc, server_message, sizeof(server_message))<=0){
                    printf("read socket not success");
                    break;
                }
                decode();
            }
            if(response.type!=JN_ACK){
                    printf("join not success\n");
                    if(response.type==JN_NAK)
                        printf("reason: %s\n", response.data);
            }
        }
        else if (strcmp(command, "/leavesession") == 0){
            // leave current session
            printf("in leavesession mode\n");

            bzero(msg.data,MAX_DATA);
            msg.type = LEAVE_SESS;
            msg.size = 0;

            encode();
            if(write(sock_desc, client_message, sizeof(client_message))<=0){
                printf("socket sent not success");
                continue;
            }

        }
        else if (strcmp(command, "/createsession") == 0){
            // /createsession <session ID> Create and join new session
            printf("in createsession mode\n");

            command  = strtok(NULL, " ");

            bzero(msg.data,MAX_DATA);
            msg.type = NEW_SESS;
            msg.size = strlen(command);
            strcpy(msg.data,command);

            encode();
            if(write(sock_desc, client_message, sizeof(client_message))<=0){
                printf("socket sent not success");
                continue;
            }
            
            if (read(sock_desc, server_message, sizeof(server_message))<=0){
                printf("read socket not success");
                continue;
            }
            decode();
            while(response.type==MESSAGE){
                text_recv();
                if (read(sock_desc, server_message, sizeof(server_message))<=0){
                    printf("read socket not success");
                    continue;
                }
                decode();
            }

            if(response.type!=NS_ACK)
                    printf("create not success\n");  

        }
        else if (strcmp(command, "/list") == 0){
            // Get the list of the connected clients and available sessions
            printf("in list mode\n");
            bzero(msg.data,MAX_DATA);
            msg.type = QUERY;
            msg.size = 0;

            encode();
            if(write(sock_desc, client_message, sizeof(client_message))<=0){
                printf("socket sent not success");
                continue;
            }
            
            if (read(sock_desc, server_message, sizeof(server_message))<=0){
                printf("read socket not success");
                continue;
            }
            decode();

            while(response.type==MESSAGE){
                text_recv();
                if (read(sock_desc, server_message, sizeof(server_message))<=0){
                    printf("read socket not success");
                    break;
                }
                decode();
            }
            if(response.type!=QU_ACK)
                    printf("get list not success\n"); 
            else{
                char * user = strtok(response.data, " ");
                while( user != NULL ) {
                    printf( "* %s\n", user );
                    user = strtok(NULL, " ");
                }
            }

        }
        else if (strcmp(command, "/quit") == 0){
            // Terminate the program
            return (0);
        }
        else{
            printf("send message: %s\n",input);
            bzero(msg.data,MAX_DATA);
            msg.type = MESSAGE;
            msg.size = strlen(input);
            strcpy(msg.data,input);

            encode();
            
            if(write(sock_desc, client_message, sizeof(client_message))<=0){
                printf("socket sent not success");
                continue;
            }
        }
    }
}
