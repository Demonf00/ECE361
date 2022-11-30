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

bool connectSocket (char * addr, char* port){
    // using default protocol->TCP
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1){
        printf("socket creation failed\n");
        return false;
    }

    // clear server socket info
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(addr);
    servaddr.sin_port = htons(atoi(port));

    // connect the client socket to server socket
    if (connect(sock_desc, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){
        printf("connection with the server failed\n");
        return false;
    }
    return true;
}


int main()
{
    bool recv_msg = false, login = false, if_connect = false,
            in_session = false;
        
    while (true)
    {
        char input[2000], cmd[2000];
        char *command;
        printf("$ ");
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
        if (strcmp(command, "/register") == 0){
            printf("in register mode\n");

            command  = strtok(NULL, " \n");
            char * name, password;
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
            
            if_connect = connectSocket(info[2],info[3]);
            if(if_connect){
                msg.type = REG;
                msg.size = strlen(info[1]);
                strcpy(msg.source,info[0]);
                strcpy(msg.data,info[1]);
            }
            else{
                printf("register not success\n");
                continue;
            }

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            printf("register request sent\n");
            read(sock_desc, server_message, sizeof(server_message));
            if(!decode()){
                printf("register failed\n");
                continue;
            }
            if(response.type!=REG_ACK){
                printf("register failed\n\n");
                continue;
            }
            setsockopt (sock_desc, SOL_SOCKET, SO_RCVTIMEO, &timeout,sizeof timeout);
            printf("registered:  %s  %s\n", info[0], info[1]);

        }

        else if (strcmp(command, "/login") == 0)
        {
            if(login){
                printf("already login. logut to switch user.\n");
                continue;
            }
            // /login <client ID> <password> <server-IP> <server-port>
            printf("in login mode\n");

            if(if_connect){
                char *info[2];
                int i=0;
                command  = strtok(NULL, " \n");
                while(command != NULL && i<2){
                    info[i++]=command;
                    command  = strtok(NULL, " \n");
                }
                if(i<2||command != NULL){
                    printf("number of agument incorrect\n");
                    continue;
                }  
                msg.size = strlen(info[1]);
                strcpy(msg.source,info[0]);
                strcpy(msg.data,info[1]);
            }
            else{
                printf("hhh\n");
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
                if(!connectSocket(info[2],info[3])){
                    printf("login not success\n");
                }       
                msg.size = strlen(info[1]);
                strcpy(msg.source,info[0]);
                strcpy(msg.data,info[1]);
            }
            msg.type = LOGIN;
            


            encode();
            write(sock_desc, client_message, sizeof(client_message));
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

            if(in_session){
                printf("already in session. leave first\n");
                continue;
            }

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
            if(response.type==JN_ASK){
                char usr_input[20];
                char * pwd;
                printf("enter password for session %s: ",command);
                fgets(usr_input,20,stdin);
                pwd =  strtok(usr_input," \n");
                
                bzero(msg.data,MAX_DATA);
                msg.type = JN_PWD;
                msg.size = strlen(pwd);
                strcpy(msg.data,pwd);

                encode();
                write(sock_desc, client_message, sizeof(client_message));
                read(sock_desc, server_message, sizeof(server_message));
                if(!decode())
                    continue;

            }
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

            if(!in_session){
                printf("not in session yet\n");
                continue;
            }

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
            if(!decode()||response.type!=NS_ASK){
                printf("create not success\n");
                continue;
            }
            bool need_pwd;
                
            while(1){
                char usr_input[20];
                char * if_pwd;
                printf("need password for session (y/n): ");
                fgets(usr_input,20,stdin);
                if_pwd =  strtok(usr_input," \n");
                if(strcmp(if_pwd,"y")==0){
                    need_pwd = true;
                    break;
                }
                if(strcmp(if_pwd,"n")==0){
                    need_pwd = false;
                    break;
                }
                printf("wrong choice entered\n");
            }
            if(need_pwd){
                char pwd[20];
                char * new_pwd;
                printf("enter password for session: ");
                fgets(pwd,20,stdin);
                new_pwd =  strtok(pwd," \n");

                bzero(msg.data,MAX_DATA);
                msg.type = NS_PWD;
                msg.size = strlen(new_pwd);
                strcpy(msg.data,new_pwd);
            }
            else{
                bzero(msg.data,MAX_DATA);
                msg.type = NS_NO;
                msg.size = 0;
            }

            encode();
            write(sock_desc, client_message, sizeof(client_message));
            
            read(sock_desc, server_message, sizeof(server_message));
            if(!decode()||response.type!=NS_ACK){
                printf("create not success\n");
                continue;
            }
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
