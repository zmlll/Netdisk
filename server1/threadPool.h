#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include <func.h>
#include "taskQueue.h"
#include <ctype.h>
#include <stdbool.h>
#include "log.h"
//主线程维持每一个子线程的信息
typedef struct threadPool_s
{

    //子线程数据
    pthread_t* tidArr;
    int workerNum;

    //任务队列
    queue_t taskQueue;

    //锁
    pthread_mutex_t mutex;

    //条件变量
    pthread_cond_t cond;

    //退出标志 1退出
    int exitflag;


}threadPool_t;

//线程池初始化
int threadPoolInit(threadPool_t *pthreadPool,int threadNum);
//创建子线程
int makeWorker(threadPool_t* pthreadPool);
//子线程的任务
void* threadFunc(void* arg);

int tcpInit(int *psockfd,const char* ip,const char* port);
int epollAdd(int epfd,int fd);
int epollDel(int epfd,int fd);
int transFile(int netfd);



#endif