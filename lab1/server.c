
// Server side implementation of UDP client-server model 
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
    if (argc != 2)
    {
        fprintf(stderr, "Missing or too many arguments!\nThe correct usage should be: server port\n");
	    exit(1);
    }

    int port = atoi(argv[1]);
    char *ftp = "ftp";
    char *yes = "yes";
    char *no = "no";
    // printf("%d\n", port);
    int sockfd; 
    char buffer[BUFSIZ]; 
    struct sockaddr_in servaddr, cliaddr; 
        
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        fprintf(stderr, "Socket creation failed!\n");
	    exit(1);
    } 
        
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
        
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(port); 
        
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        fprintf(stderr, "Bind failed!\n");
	    exit(1);
    } 
        
    int len, n; 
    
    len = sizeof(cliaddr);  //len is value/result 
    
    printf("Server: Start listening to port %d ...\n", port);
    n = recvfrom(sockfd, (char *)buffer, BUFSIZ,  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Server: Received message from %d : %s\n", cliaddr.sin_addr.s_addr, buffer); 

    if (strcmp(buffer, ftp) != 0)
    {
        sendto(sockfd, (const char *)no, strlen(no),  
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
            len);
        printf("Server: %s message sent.\n", no);
    } 

    else
    {
        sendto(sockfd, (const char *)yes, strlen(yes),  
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
            len);
        printf("Server: %s message sent.\n", yes);
    } 
      
        
    return 0; 
}