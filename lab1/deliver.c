// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h>
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
    char command[BUFSIZ];
    char filepath[BUFSIZ];
    char *ftp = "ftp";
    char *yes = "yes";
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
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); 
        
    int n, len; 
    
    printf("Please input a message as follows: ftp <file name>\n");
    scanf("%s", command);
    scanf("%s", filepath);
 
    if (strcmp(command, ftp)!=0)
    {
        fprintf(stderr, "Wrong command! Please input as follows: ftp <filename>\n");
        exit(1);
    }

    int file = open(filepath, O_RDONLY);
    if (file < 0)
    {
        fprintf(stderr, "File %s does not exist!\n", filepath);
	    exit(1);
    }

    sendto(sockfd, (const char *)ftp, strlen(ftp), 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)); 
    // printf("Hello message sent.\n");
            
    n = recvfrom(sockfd, (char *)buffer, BUFSIZ,  
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len); 
    buffer[n] = '\0';
    if (strcmp(buffer, yes) == 0) 
        printf("A file transfer can start\n");

    else{
        fprintf(stderr, "Please try again!\n");
	    exit(1);
    } 
    
    close(sockfd); 
    return 0; 
}