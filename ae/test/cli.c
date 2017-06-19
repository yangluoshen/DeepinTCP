#include "anet.h"
#include "ae.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>

#define MAX_BUF_SIZE (1024)
#define MAX_EV_NUM (100)

int sockfd = -1;
struct aeEventLoop* cli_el;
int family = AF_LOCAL;

int init_sock();

void free_conn(int fd, int mask)
{
    aeDeleteFileEvent(cli_el, fd, mask);
    close(fd);
    sockfd = -1;
}

void read_sock(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    (void)eventLoop; (void)clientData;
    char buf[MAX_BUF_SIZE] = {0};
    ssize_t n = read(fd, buf, MAX_BUF_SIZE);
    if (n<0){
        if (errno == EAGAIN || errno == EINTR) return;
        perror("read server");
        free_conn(fd, mask);
        return;
    }else if(n == 0){
        printf("WARNING: server has been closed. type \"rc\" to reconnect.\n");
        free_conn(fd, mask);
        return;
    }

    write(fileno(stdout), buf, n);
}

void stdin_handle(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    (void)eventLoop;(void)clientData;
    char buf[MAX_BUF_SIZE] = {0};
    ssize_t n = read(fd, buf, MAX_BUF_SIZE);
    if (n<0){
        if (errno==EAGAIN || errno==EINTR) return;
        perror("stdin read");
        return;
    }

    // "n-1" consider the final character "\n"
    if (strncmp(":bye", buf, n-1) == 0){
        printf("Disconnect! \n");
        free_conn(sockfd, AE_READABLE);
        return;
    }
    else if (strncmp(":rc", buf, n-1) == 0){
        if (ANET_ERR == init_sock())
            printf("reconnect failed\n");
        else
            printf("OK!\n");
        return;
    }else
        write(sockfd, buf, n);

    return;
}

int init_sock()
{
    if (-1 != sockfd) return ANET_ERR;
    if (AF_LOCAL == family){
        sockfd = anetUnixConnect(NULL, "/opt/unixsock");
    }else if (AF_INET == family){
        sockfd = anetTcpNonBlockConnect(NULL, "127.0.0.1", 5730);
    }else{
        printf("error unknown family[%d]\n", family);
        sockfd = ANET_ERR;
    }
    if (sockfd == ANET_ERR){
        perror("connect");
        exit(0);
    }

    aeCreateFileEvent(cli_el, sockfd, AE_READABLE, read_sock, NULL);
    return sockfd;
}

void init_cli()
{
    cli_el = aeCreateEventLoop(MAX_EV_NUM);
    if (ANET_ERR == init_sock()) exit(0);

    aeCreateFileEvent(cli_el, fileno(stdin), AE_READABLE, stdin_handle, NULL);
}

int main(int argc, char** argv)
{
    if (argc == 2 && 0 == strcasecmp(argv[1], "inet")){
        family = AF_INET;
    }

    init_cli();

    aeMain(cli_el);

    return 0;
}


