#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];
    int server_struct_length = sizeof(server_addr);
    
    // deliever ip_address port
    if (argc != 3) {
        printf("argument format incorrect  %d\n", argc);
        exit(1);
    }
    
    // Create IPv4 UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    
    // Set port and IP:
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    printf("deliver <%s> <%d>\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    while(1){
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));    
        
    // Get input from the user:
    printf("Enter 'ftp <file name>' ");
    gets(client_message);
    
    // check input format and if file exist
    char *message = strtok(client_message, " ");
    if(strcmp(message,"ftp")==0 ){
        message = strtok(NULL, " ");
        if(access(message, F_OK)!=0){
            printf("file not exist. \n");
            continue; 
        }            
    }       
    else{
        printf("Incorrect format.\n");
        continue;
    }
        
    
    // Send the message to server:
    if(sendto(socket_desc, "ftp", strlen("ftp"), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    printf("ftp sent to server\n");
    // Receive the server's response:
    recvfrom(socket_desc, server_message, sizeof(server_message), 0,(struct sockaddr*)&server_addr, &server_struct_length);
    
    if(strcmp(server_message,"yes")==0)
        printf("A file transfer can start\n");
    else
        break;
    }

}

