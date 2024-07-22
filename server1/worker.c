#include "threadPool.h"

void *threadFunc(void* arg)
{
    threadPool_t* pthreadPool = (threadPool_t*)arg;
    while(1)
    {
        //取出任务
        pthread_mutex_lock(&pthreadPool->mutex);
        while(pthreadPool->taskQueue.queueSize == 0)
        {
            pthread_cond_wait(&pthreadPool->cond,&pthreadPool->mutex);
        }
        printf("worker got a task!\n");
        int netfd = pthreadPool->taskQueue.pfront->fd;
        Dequeue(&pthreadPool->taskQueue);
        pthread_mutex_unlock(&pthreadPool->mutex);
        transFile(netfd);
        close(netfd);
    } 
}

int makeWorker(threadPool_t* pthreadPool)
{
    for(int i = 0;i<pthreadPool->workerNum;i++)
    {
        pthread_create(&pthreadPool->tidArr[i],NULL,threadFunc,pthreadPool);
    }
    return 0;
}