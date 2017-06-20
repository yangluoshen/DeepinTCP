#include <sys/epoll.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>

#include "ae.h"

#define MAX_BUF_SIZE  (1024)

event_loop* ae_create()
{
    event_loop* el;
    if ((el = (event_loop*) malloc(sizeof(event_loop))) == NULL) goto err;
    el->setsize = DEFAULT_SETSIZE;
    if ((el->events = (file_event*) malloc(el->setsize*sizeof(file_event))) == NULL) goto err;
    if ((el->fired = (fired_event*) malloc(el->setsize*sizeof(fired_event))) == NULL) goto err;
    
    el->maxfd = -1;
    el->stop = 0;
    el->epfd = epoll_create(1024);

    int i;
    for (i = 0; i < el->setsize; ++i){
        el->events[i].mask = AE_NONE;
    }

    return el;
err:
   if (el){
       free (el->events);
       free (el->fired);
       free (el);
   } 
   return NULL;
}

int ae_resize_setsize(event_loop* el, int setsize)
{
    (void)el;(void)setsize;
    return 65535;
}

int ae_add_poll_event(event_loop* el, int fd, int mask)
{
    if (fd > el->setsize-1) return -1;
    int op = el->events[fd].mask == AE_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    struct epoll_event ev;
    ev.events = 0;
    mask |= el->events[fd].mask;
    if (mask & AE_READABLE) ev.events |= EPOLLIN;
    if (mask & AE_WRITABLE) ev.events |= EPOLLOUT;

    ev.data.fd = fd;
    if (-1 == epoll_ctl(el->epfd, op, fd, &ev)) return AE_ERR;

    return AE_OK;
}

int ae_del_poll_event(event_loop* el, int fd, int mask)
{
    if (fd > el->setsize-1) return AE_ERR;
    mask |= el->events[fd].mask & (~mask);

    if (mask == AE_NONE){
        epoll_ctl(el->epfd, EPOLL_CTL_DEL, fd, NULL);
    }
    else{
        struct epoll_event ev;
        ev.events = 0;
        if (mask & AE_READABLE) ev.events |= EPOLLIN;
        if (mask & AE_WRITABLE) ev.events |= EPOLLOUT;
        epoll_ctl(el->epfd, EPOLL_CTL_MOD, fd, &ev);
    }

    return AE_OK;
}

int ae_poll(event_loop* el, struct timeval* tvp)
{
    const int MAX_JOBS_IN_APOLL = 1024;
    struct epoll_event evs[MAX_JOBS_IN_APOLL];
    
    int ready = epoll_wait(el->epfd, evs, MAX_JOBS_IN_APOLL, tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000) : 1000);

    int i;
    for (i = 0; i < ready; ++i){
        struct epoll_event* ev = evs + i;
        int mask = 0;
        if (ev->events & EPOLLIN)  mask |= AE_READABLE;
        if (ev->events & EPOLLOUT) mask |= AE_WRITABLE;
        if (ev->events & EPOLLERR) mask |= AE_WRITABLE;
        if (ev->events & EPOLLHUP) mask |= AE_WRITABLE;

        el->fired[i].fd = ev->data.fd;
        el->fired[i].mask = mask;
    }

    return ready;
}

int ae_create_file_event(event_loop* el, int fd, int mask, ae_file_proc proc, void* data)
{
    if (-1 == ae_add_poll_event(el, fd, mask)) return AE_ERR;

    file_event* fe = & el->events[fd];
    if (mask & AE_READABLE) fe->read_proc = proc;
    if (mask & AE_WRITABLE) fe->write_proc = proc;
    fe->mask |= mask;
    fe->data = data;

    if (el->maxfd < fd) el->maxfd = fd;
    
    return AE_OK;
}

int ae_delete_file_event(event_loop* el, int fd, int mask)
{
    if (AE_ERR == ae_del_poll_event(el, fd, mask)) return AE_ERR;
    file_event* fe = & el->events[fd];
    fe->mask &= ~mask;
    if (fe->mask == AE_NONE && el->maxfd == fd){
        int i;
        for (i = fd; i >=0; --i){
            if (el->events[i].mask != AE_NONE) break;
        }
        el->maxfd = i;
    }
    
    return AE_OK;
}

int ae_main(event_loop* el)
{
    if (!el) return AE_ERR;

    while(!el->stop){
        int job_num = ae_poll(el, NULL);
        int i;
        for (i = 0; i < job_num; ++i){
            int fd = el->fired[i].fd;
            int mask = el->fired[i].mask;
            file_event* fe = & el->events[fd];
            int rfired = 0;
            if ((fe->mask & mask & AE_READABLE) && fe->read_proc){
                fe->read_proc(el, fd, mask, fe->data);
                rfired = 1;
            }

            if (!rfired && (fe->mask & mask & AE_WRITABLE) && fe->write_proc){
                fe->write_proc(el, fd, mask, fe->data);
            }
        } 
    }

    return AE_OK;
}

#ifdef __AE_MAIN
#include <errno.h>
#include <stdlib.h>

void do_echo(event_loop* el, int fd, int mask, void* data)
{
    char buf[MAX_BUF_SIZE] = {0};
    ssize_t n = read(fd, buf, MAX_BUF_SIZE);
    if (n < 0 && errno == EINTR) return;
    else if (n < 0) {
        perror("read");
        return;
    }
    else if (n == 0) exit(0);

    write(STDOUT_FILENO, buf, n);

    (void)el; (void)mask; (void)data;
}

void init(event_loop* el)
{
    if (AE_ERR == ae_create_file_event(el, STDIN_FILENO, AE_READABLE, do_echo, NULL)){
        printf("create file event failed\n");
        exit(0);
    }
}

int main(int argc, char** argv)
{
    event_loop* el = ae_create();

    init(el);
    ae_main(el);

    return 0;
}

#endif

