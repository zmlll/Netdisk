#ifndef __TASK_QUEUE__
#define __TASK_QUEUE__

#include <func.h>

typedef struct node_s
{
    int fd;
    struct node_s* pnext;
}node_t;

typedef struct queue_s
{
    node_t* pfront;
    node_t* prear;
    int queueSize;
}queue_t;

int Enqueue(queue_t* pqueue,int fd);
int Dequeue(queue_t* pqueue);
#endif