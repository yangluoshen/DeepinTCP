#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVPORT 9527 

int main(int argc, char** argv)  
{  
    int listen_fd, real_fd;  
    struct sockaddr_in listen_addr, client_addr;  
    socklen_t len = sizeof(struct sockaddr_in);  
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);  
    if(listen_fd == -1) {  
        perror("socket failed");  
        return -1;  
    }  
    bzero(&listen_addr,sizeof(listen_addr));  
    listen_addr.sin_family = AF_INET;  
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    listen_addr.sin_port = htons(SERVPORT);  
    bind(listen_fd,(struct sockaddr *)&listen_addr, len);  
    listen(listen_fd, 50);  

    while(1){  
        real_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len);  
        if(real_fd == -1)  {  
            perror("accpet failed");  
            return -1;  
        }  
        if(fork() == 0)  {  
            close(listen_fd);  
            char pcContent[1024];
            read(real_fd,pcContent,1024);
            printf("connected\n");
            close(real_fd);  
            exit(0);              
        }  
        close(real_fd);  
    }     
    return 0;  
}


