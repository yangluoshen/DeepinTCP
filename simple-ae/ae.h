#ifndef __AE_H
#define __AE_H
#include <sys/time.h>

#define AE_NONE (0)
#define AE_READABLE (1)
#define AE_WRITABLE (2)

#define AE_OK (0)
#define AE_ERR (-1)

#define DEFAULT_SETSIZE (10240)

struct event_loop;
typedef void (*ae_file_proc)(struct event_loop* el, int fd, int mask, void* data);

typedef struct file_event{
    int mask;
    ae_file_proc read_proc;
    ae_file_proc write_proc;
    void *data;
}file_event;

typedef struct fired_event{
    int fd;
    int mask;
}fired_event;

typedef struct event_loop{
    int epfd;
    int maxfd;
    int setsize;
    file_event* events;
    fired_event* fired;
    int stop;
}event_loop;

event_loop* ae_create(void);
int ae_add_poll_event(event_loop* el, int fd, int mask);
int ae_del_poll_event(event_loop* el, int fd, int delmask);
int ae_poll(event_loop* el, struct timeval* tvp);

int ae_resize_setsize(event_loop* el, int setsize);
int ae_create_file_event(event_loop* el, int fd, int mask, ae_file_proc proc, void* data);
int ae_delete_file_event(event_loop* el, int fd, int mask);
int ae_main(event_loop* el);

#endif


