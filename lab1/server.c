#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int main(int argc, char *argv[]){
    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];
    int client_struct_length = sizeof(client_addr);
    
    // server ip_address port
    if (argc != 3) {
        printf("argument format incorrect  %d\n", argc);
        return 1;
    }
    
    // Create UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    
    printf("server <%d>\n",ntohs(server_addr.sin_port));
    
//    while(1){
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    // Receive client's message:
    recvfrom(socket_desc, client_message, sizeof(client_message), 0,(struct sockaddr*)&client_addr, &client_struct_length);
 
    printf("Received message from IP: %s and port: %d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(atoi(client_addr.sin_port)));
            
    printf("Msg from client: %s\n", client_message);
    
    // Respond to client:
    if(strcmp("ftp",client_message)==0)
        strncpy(server_message, "yes",2000);
    else
        strncpy(server_message, "no",2000);

    sendto(socket_desc, server_message, strlen(server_message), 0,(struct sockaddr*)&client_addr, client_struct_length);
    
//    }
}
