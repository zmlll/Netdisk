#include "taskQueue.h"

int Enqueue(queue_t* pqueue,int fd)
{
    node_t* pnew = (node_t*)calloc(1,sizeof(node_t));
    pnew->fd = fd;
    if(pqueue->queueSize == 0)
    {
        pqueue->pfront = pnew;
        pqueue->prear = pnew;
    }
    else
    {
        pqueue->prear->pnext = pnew;
        pqueue->prear = pnew;
    }       
    pqueue->queueSize++;
    return 0;
}

int Dequeue(queue_t* pqueue)
{
    node_t* pcur = pqueue->pfront;
    pqueue->pfront = pcur->pnext;
    free(pcur);
    if(pqueue->queueSize == 1)
    {
        pqueue->prear = NULL;
    } 
    --pqueue->queueSize;
    return 0;
}