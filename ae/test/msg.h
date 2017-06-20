#ifndef __MSG_H
#define __MSG_H

typedef struct msg_t{
    int whoami;
    int len;
    char data[];
}msg_t;

#endif
