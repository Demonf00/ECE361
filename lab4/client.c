#include "conference.h"

#define MAX_CMD 80

int sock_desc, connfd;
struct sockaddr_in servaddr;
struct message msg, response;
char server_message[5000], client_message[5000];
// char * received_msg[15];
// front = -1;
// rear = -1;

// void enqueue(char *insert_item)
// {
//     int insert_item;
//     if (Rear <15){
//         if (Front == - 1)
//             Front = 0;
//         strcpy(response.data,contain[3])
//         received_msg[++Rear] = insert_item;
//     }
// } 

// void show()
// {
//     if (Front != - 1){
//         for (int i = Front; i <= Rear; i++)
//             printf("%d ", inp_arr[i]);
//         printf("\n");
//     }
// } 

void encode(){
    bzero(client_message,sizeof(client_message));
    snprintf(client_message, sizeof(client_message),
    "%d;%d;%s;%s",msg.type,msg.size,msg.source,msg.data);
}
bool decode(){
    bzero(response.data,MAX_DATA);
    char *item  = strtok(server_message, ";");
    char *contain[4];
    int i=0;
    while(item != NULL){
        contain[i++]=item;
        item  = strtok(NULL, ";");
    }
    if (i<4){
        // printf("decode not success\n");
        return false;
    }
    response.type = atoi(contain[0]);
    response.size = atoi(contain[1]);
    strcpy(response.source,contain[2]);
    strcpy(response.data,contain[3]);

    bzero(server_message,sizeof(server_message));
    return true;

}

void text_recv(){
    printf("- received message: %s\n",response.data);
}

void read_msg(){
    while(response.type==MESSAGE){
        text_recv();
        read(sock_desc, server_message, sizeof(server_message));
        if(!decode())
            return;
    }
}

int main()
{
    bool recv_msg = false;

    while (true)
    {
        char input[2000], cmd[2000];
        if(recv_msg){
            if(read(sock_desc, server_message, sizeof(server_message))>0){
                if(decode()){
                    if(response.type==MESSAGE)
                        text_recv();
                }
            }
        }
        printf("$ ");
        fgets(input, 2000, stdin);
        
        strcpy(cmd,input);
        char *command = strtok(cmd, " \n");
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;

        if (strcmp(command, "/login") == 0)
        {
            // /login <client ID> <password> <server-IP> <server-port>
            printf("in login mode\n");

            char *info[4];
            int i=0;
            command  = strtok(NULL, " ");
            while(command != NULL&&i<4){
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
                continue;
            }

            
            msg.type = LOGIN;
            msg.size = strlen(info[1]);
            strcpy(msg.source,info[0]);
            strcpy(msg.data,info[1]);


            encode();
            int xx = write(sock_desc, client_message, sizeof(client_message));
            printf("longin request sent\n");
            read(sock_desc, server_message, sizeof(server_message));
            if(!decode()){
                printf("login not success\n");
                continue;
            }
            if(response.type!=LO_ACK){
                    printf("login not success\n");
                    // if(response.type==LO_NAK)
                    //     printf("%s\n", response.data);
                    continue;
            }
            setsockopt (sock_desc, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof timeout);
            printf("login success\n");
        }
        else if (strcmp(command, "/logout") == 0){
            //reset client
            printf("in logout mode\n");

            bzero(msg.data,MAX_DATA);
            msg.type = EXIT;
            msg.size = 0;

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            
       
            // continue;
        }
        else if (strcmp(command, "/joinsession") == 0){
            // /joinsession <session ID>
            read(sock_desc, server_message, sizeof(server_message));
            if(decode()){
                read_msg();
            }
            
            printf("in joinsession mode\n");
            command  = strtok(NULL, " \n");
            char *session_id = command;

            bzero(msg.data,MAX_DATA);
            msg.type = JOIN;
            msg.size = strlen(command);
            strcpy(msg.data,command);

            

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            
            read(sock_desc, server_message, sizeof(server_message));
            if(!decode())
                continue;
            read_msg();
            if(response.type!=JN_ACK){
                    printf("join not success\n");
                    if(response.type==JN_NAK)
                        printf("reason: %s\n", response.data);
                    continue;
            }
            printf("joined session %s\n", command);
            recv_msg = true;                
            
        }
        else if (strcmp(command, "/leavesession") == 0){
            // leave current session
            printf("in leavesession mode\n");

            bzero(msg.data,MAX_DATA);
            msg.type = LEAVE_SESS;
            msg.size = 0;

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            printf("leaved session\n");
            recv_msg = false;

        }
        else if (strcmp(command, "/createsession") == 0){
            // /createsession <session ID> Create and join new session
            read(sock_desc, server_message, sizeof(server_message));
            if(decode()){
                read_msg();
            }
            printf("in createsession mode\n");

            command  = strtok(NULL, " \n");

            bzero(msg.data,MAX_DATA);
            msg.type = NEW_SESS;
            msg.size = strlen(command);
            strcpy(msg.data,command);

            

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            
            read(sock_desc, server_message, sizeof(server_message));
            if(!decode())
                continue;
            read_msg();

            if(response.type!=NS_ACK)
                printf("create not success\n");  
            else
                printf("created session %s\n", command);
            

        }
        else if (strcmp(command, "/list") == 0){
            // Get the list of the connected clients and available sessions
            read(sock_desc, server_message, sizeof(server_message));
            if(decode()){
                read_msg();
            }
            printf("in list mode\n");
            bzero(msg.data,MAX_DATA);
            msg.type = QUERY;
            msg.size = 0;

            

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            
            read(sock_desc, server_message, sizeof(server_message));
            if(!decode())
                continue;

            read_msg();
            if(response.type!=QU_ACK){
                printf("get list not success %d\n", response.type); 
            }
                    
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
            // bzero(msg.data,MAX_DATA);
            // msg.type = QUIT;
            // msg.size = 0;

            // encode();
            // write(sock_desc, client_message, sizeof(client_message));
            close(sock_desc);
            return (0);
        }
        else{
            char *send_msg = strtok(input, "\n");
            printf("send message: %s\n",send_msg);
            bzero(msg.data,MAX_DATA);
            msg.type = MESSAGE;
            msg.size = strlen(send_msg);
            strcpy(msg.data,send_msg);

            encode();
            printf("send %s\n", client_message);
            write(sock_desc, client_message, sizeof(client_message));
        }
        
            
    }
}
