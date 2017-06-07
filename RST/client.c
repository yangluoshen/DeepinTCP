#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)  
{  
    int s;  
    struct sockaddr_in s_addr;  
    socklen_t len = sizeof(s_addr);  
    s = socket(AF_INET, SOCK_STREAM, 0);  
    if(s == -1)  {  
        perror("socket failed  ");  
        return -1;  
    }  
    s_addr.sin_family = AF_INET;  
    s_addr.sin_port = htons(9527);  
    s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(s,(struct sockaddr*)&s_addr,len) == -1)  {  
        perror("connect fail  ");  
        return -1;  
    }  
    char pcContent[1030]={0};
    write(s,pcContent,1030);
    sleep(1);
    close(s);
}

