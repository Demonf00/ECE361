#include "udp.h"

int main(int argc, char *argv[]){
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000], data[1000];
    int server_struct_length = sizeof(server_addr);
    struct timeval start_time, end_time;
    double diff_time;
    
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
    
    // Test if can send the message to server:
    gettimeofday(&start_time, NULL);
    if(sendto(socket_desc, "ftp", strlen("ftp"), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    
    recvfrom(socket_desc, server_message, sizeof(server_message), 
            0,(struct sockaddr*)&server_addr, &server_struct_length);

    
    if(strcmp(server_message,"yes")==0)
        printf("A file transfer can start\n");
    else
        break;
    
    
    // Get input from the user:
    printf("Enter 'ftp <file name>' ");
    fgets(client_message,2000,stdin);
    int change = strlen(client_message);
    client_message[change - 1] = '\0';
    
    // check input format and if file exist
    char *message = strtok(client_message, " ");
    if(strcmp(message,"ftp")==0 ){
        message = strtok(NULL, " ");
        if(open(message,O_RDONLY) < 0){
            printf("file not exist. \n");
            continue; 
        }
    }       
    else{
        printf("Incorrect format.\n");
        continue;
    }
    
    recvfrom(socket_desc, (struct timeval *)&end_time, sizeof(end_time), 
            0,(struct sockaddr*)&server_addr, &server_struct_length);
    // printf("Received time from server: %6.6f\n", end_time);
    diff_time = (double)(end_time.tv_usec - start_time.tv_usec) / 1000000 + (double)(end_time.tv_sec - start_time.tv_sec);
    // diff_time = end_time - start_time;
    printf("Use %lfs from client to server\n", diff_time);

    // Check total frag needed
    FILE * file = fopen(message,"r");
    fseek(file, 0 , SEEK_END);
    double filesize = (double)ftell(file);
    fseek(file, 0 , SEEK_SET);  // Reset to beginning of file
    int total_frag = (int)ceil(filesize/1000);
    int frag_no = 1;

        
    
    
    // Send file to server
    struct packet file_frag;
    file_frag.total_frag = total_frag;
    strcpy(file_frag.filename, message);
    // file_frag.filename = message;
    
        
    while(fread(&data, sizeof(char), 1000,file) > 0){
        file_frag.frag_no = frag_no;
        file_frag.size = strlen(data);
        memcpy(file_frag.filedata,data,1000);
        
        if(sendto(socket_desc, (struct packet*)&file_frag, sizeof(file_frag), 0,
         (struct sockaddr*)&server_addr, server_struct_length) < 0){
            printf("Unable to send packet # %d\n",file_frag.frag_no);
            continue;
        }
        printf("packet %d of %d sent\n", file_frag.frag_no,file_frag.total_frag);
//        printf("packet contains: \n%s \n", file_frag.filedata);
        
        bzero(data,1000); // clear buffer
        bzero(file_frag.filedata,1000);
        
        do{
            bzero(server_message,2000);
            recvfrom(socket_desc, server_message, sizeof(server_message), 
                0,(struct sockaddr*)&server_addr, &server_struct_length);
        }while(strcmp(server_message,"yes")!=0);
                
        frag_no++;
    }    
        
    
    printf("file sent to server\n");
    break;
    // // Receive the server's response:
    // recvfrom(socket_desc, server_message, sizeof(server_message), 
    //         0,(struct sockaddr*)&server_addr, &server_struct_length);
    



  

    }

    

}

