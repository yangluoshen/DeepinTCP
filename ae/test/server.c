#include "ae.h"
#include "anet.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

#define MAX_EV_NUM (100)
#define MAX_BUF_SIZE (1024)
#define UNIX_SOCK_PATH "/opt/unixsock"

struct {
    int sv_fd;
    int port;
    int backlog;
    aeEventLoop* el;
    char neterr[256];

} server;

void free_conn(int fd, int mask)
{
    aeDeleteFileEvent(server.el, fd, mask);
    close(fd);
}

// do echo
void read_client(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    (void)eventLoop;(void)clientData;
    char buf[MAX_BUF_SIZE] = {0};
    ssize_t n = read(fd, buf, MAX_BUF_SIZE);
    if (n<0){
        if (errno == EAGAIN || errno == EINTR) return;
        perror("read client");
        free_conn(fd, mask);
        return;
    }else if (n==0){
        printf("client[%d] leave\n", fd);
        free_conn(fd, mask);
        return;
    }
    
    printf("recv:%s\n", buf);
    if (n != write(fd, buf, n)){
        perror("echo failed");
        return;
    }
}

void accept_handle(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask)
{
    (void)eventLoop;(void)clientData;(void)mask;
    int conn_fd = anetTcpAccept(server.neterr, fd, NULL, 0, NULL); 
    if (ANET_ERR == conn_fd) return;

    anetNonBlock(NULL, conn_fd);
    if (ANET_ERR == aeCreateFileEvent(server.el, conn_fd, AE_READABLE, read_client, NULL)){
        exit(0);
    }

    printf("client[%d] join\n", conn_fd);
}

void unix_accept_handle(struct aeEventLoop* el, int fd, void *clientData, int mask){
    (void) el; (void)clientData; (void)mask;

    int conn_fd = anetUnixAccept(server.neterr, fd);
    if (ANET_ERR == conn_fd) return;

    anetNonBlock(NULL, conn_fd);

    if (ANET_ERR == aeCreateFileEvent(server.el, conn_fd, AE_READABLE, read_client, NULL)) exit(0);

    printf("client[%d] join\n", conn_fd);
}

void init_server(int family)
{
    server.el = aeCreateEventLoop(MAX_EV_NUM);
    server.backlog = 100;

    if (family == AF_INET){
        server.port = 5730;
        server.sv_fd = anetTcpServer(server.neterr, server.port, NULL, server.backlog); 
        if (ANET_ERR == server.sv_fd) exit(1);

        anetNonBlock(NULL, server.sv_fd);

        if (ANET_ERR == aeCreateFileEvent(server.el, server.sv_fd, AE_READABLE, accept_handle, NULL)){
            exit(0);
        }
    }else if (AF_LOCAL == family){
        unlink(UNIX_SOCK_PATH);
        server.sv_fd = anetUnixServer(server.neterr, UNIX_SOCK_PATH, 0777, server.backlog);
        if (ANET_ERR == server.sv_fd) {
            printf("Err: %s\n", server.neterr);
            exit(0);
        }

        anetNonBlock(NULL, server.sv_fd);
        if (ANET_ERR == aeCreateFileEvent(server.el, server.sv_fd, AE_READABLE, unix_accept_handle, NULL)) exit(0);
        
    }
    else {
        printf("unknow family[%d]\n", family);
    }

    return;
}

int main(int argc, char** argv)
{
    int family = AF_LOCAL;
    if (argc == 2 && 0 == strcasecmp(argv[1], "inet")){
        family = AF_INET;
    }

    init_server(family);

    aeMain(server.el);

    return 0;
}
