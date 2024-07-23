#include "threadPool.h"

int threadPoolInit(threadPool_t* pthreadPool,int threadNum)
{
    //这里一定要先清零，尤其是队列的长度，否则会出错
    //flag在这里也是设置为0，所以必须是0不退出，收到父进程的信号后flag置为1才退出
    bzero(pthreadPool,sizeof(threadPool_t));
    pthreadPool->workerNum = threadNum;
    pthreadPool->tidArr = (pthread_t*)calloc(threadNum,sizeof(pthread_t));
    pthread_mutex_init(&pthreadPool->mutex,NULL);
    pthread_cond_init(&pthreadPool->cond,NULL);
    return 0;
}