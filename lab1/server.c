
// Server side implementation of UDP client-server model 

#include "udp.h" 
    
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
    struct timeval end_time;
    struct packet recvpacket;

    //set timeout for socket
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000; //avg 480ms for roundtime
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
        perror("Error");
    } 
        
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
    
    gettimeofday(&end_time, NULL);
    // printf("%d\n", start_time);

    buffer[n] = '\0'; 
    printf("Server: Received message from %s: %s\n", inet_ntoa(cliaddr.sin_addr), buffer); 

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
    


    // sendto(sockfd, (const struct timeval *)&end_time, sizeof(end_time),  
    //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
    //         len);
    
    // printf("Server: end time sent.\n");

    while(recvfrom(sockfd, (struct packet*)&recvpacket, sizeof(struct packet),  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &len)<0);

    creat(recvpacket.filename, S_IRWXU);
    int write_to_file = open(recvpacket.filename, O_WRONLY);
    write(write_to_file, recvpacket.filedata, recvpacket.size);
    printf("Server: recv packet from client %d of %d\n", recvpacket.frag_no, recvpacket.total_frag);
    
    while(sendto(sockfd, (const char *)yes, strlen(yes),  
        MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
            len)<0);

    //simulate loss
    int if_drop = 0;
    int previous = -1;
    while(recvpacket.frag_no != recvpacket.total_frag)
    {
        // sendto(sockfd, (const char *)yes, strlen(yes),  
        // MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
        //     len);
        if (recvfrom(sockfd, (struct packet*)&recvpacket, sizeof(struct packet),  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &len)<0)
            {
                sendto(sockfd, (const char *)no, strlen(no),  
                    MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
                    len);
                continue;
            }
        if (recvpacket.frag_no == previous)
        {
            printf("Server: recv packet from client %d of %d, already got it, dropped!\n", recvpacket.frag_no, recvpacket.total_frag);
            while(sendto(sockfd, (const char *)yes, strlen(yes),  
                    MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
                    len)<0);
            continue;
        }
        previous = recvpacket.frag_no;
        write(write_to_file, recvpacket.filedata, recvpacket.size);
        

        printf("Server: recv packet from client %d of %d\n", recvpacket.frag_no, recvpacket.total_frag);
        if(if_drop%47!=0){
            printf("Server: ACK sent!\n");
            while(sendto(sockfd, (const char *)yes, strlen(yes),  
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
                len)<0);
            
        }
        else printf("Server: Simulate: ACK dropped!\n");
        if_drop++;
        
        
    }

    close(write_to_file);
    // printf("hello\n");
    // printf("%d\n", strlen(recvpacket.filename));
    printf("Server: finished transfer the file %s\n", recvpacket.filename);

    return 0; 
}
