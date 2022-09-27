// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
    
    
// Driver code 
int main(int argc, char *argv[]) {
    if (argc != 3)
    {
        fprintf(stderr, "Missing or too many arguments!\nThe correct usage should be: deliver address port\n");
	    exit(1);
    }

    int port = atoi(argv[2]);
    // printf("%d\n", port);
    int sockfd; 
    char buffer[BUFSIZ]; 
    char *hello = "Hello from client"; 
    struct sockaddr_in     servaddr; 
    
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        fprintf(stderr, "Socket error!\n");
	    exit(1);
    } 
    
    memset(&servaddr, 0, sizeof(servaddr)); 
        
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
        
    int n, len; 
        
    sendto(sockfd, (const char *)hello, strlen(hello), 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)); 
    printf("Hello message sent.\n"); 
            
    n = recvfrom(sockfd, (char *)buffer, BUFSIZ,  
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Server : %s\n", buffer); 
    
    close(sockfd); 
    return 0; 
}