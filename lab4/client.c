#include "conference.h"

#define MAX_CMD 80

int sock_desc=-1, connfd;
struct sockaddr_in servaddr;
struct message msg, response;
char server_message[5000], client_message[5000];
 

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


int main()
{
    bool recv_msg = false;
    bool login = false;
        
    while (true)
    {
        char input[2000], cmd[2000];
        char *command;
        printf("$");
        fflush(stdout);
        
        struct timeval timeout;
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;

        int sck, n;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        sck = sock_desc;    
        FD_SET(sck, &read_fds);
        FD_SET(fileno(stdin), &read_fds);
        n = ((sck) >= (fileno(stdin))) ? (sck) : (fileno(stdin));
        
        select(n+1, &read_fds, NULL,NULL,NULL);
        
        
        if (FD_ISSET(sck, &read_fds)) {
            read(sock_desc, server_message, sizeof(server_message));
            if (recv_msg){
                if(decode())
                    printf("- received message: %s\n",response.data);
            }
            bzero(server_message,sizeof(server_message));
            continue;
        }
        if (FD_ISSET(fileno(stdin), &read_fds)) {
            read(fileno(stdin), input, 2000);
            strcpy(cmd,input);
            command = strtok(cmd, " \n");
        }

        if (strcmp(command, "/login") == 0)
        {
            if(login){
                printf("already login. logut to switch user.\n");
            }
            // /login <client ID> <password> <server-IP> <server-port>
            printf("in login mode\n");

            char *info[4];
            int i=0;
            command  = strtok(NULL, " \n");
            while(command != NULL && i<4){
                info[i++]=command;
                command  = strtok(NULL, " \n");
            }
            if(i<4||command != NULL){
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
            login = true;  
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
        else if (!login){
            printf("not logined\n");
            continue;
        }
        else if (strcmp(command, "/logout") == 0){
            //reset client
            printf("logout now\n");

            bzero(msg.data,MAX_DATA);
            msg.type = EXIT;
            msg.size = 0;

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            login = false;
            recv_msg = false;

       
            // continue;
        }
        else if (strcmp(command, "/joinsession") == 0){
            
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
            if(response.type!=JN_ACK){
                    printf("join not success\n");
                    // if(response.type==JN_NAK)
                    //     printf("reason: %s\n", response.data);
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

            if(response.type!=NS_ACK)
                printf("create not success\n");  
            else
                printf("created session %s\n", command);
            

        }
        else if (strcmp(command, "/list") == 0){
            printf("in list mode\n");
            bzero(msg.data,MAX_DATA);
            msg.type = QUERY;
            msg.size = 0;

            

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            
            read(sock_desc, server_message, sizeof(server_message));
            if(!decode())
                continue;

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
        
        else if (recv_msg){
            char *send_msg = strtok(input, "\n");
            printf("send message: %s\n",send_msg);
            bzero(msg.data,MAX_DATA);
            msg.type = MESSAGE;
            msg.size = strlen(send_msg);
            strcpy(msg.data,send_msg);

            encode();
            write(sock_desc, client_message, sizeof(client_message));
        }
        else
            printf("not in session\n");
        
            
    }
}
