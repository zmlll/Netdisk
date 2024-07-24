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

enum cmd_t
{
    CD,
    LS,
    PUTS,
    GETS,
    REMOVE,
    PWD,
    MKDIR,
    RMDIR,
    INVALID
};

typedef struct train_s{
    int length;
    char data[1000];
}train_t;

struct sockaddr_in clientAddr;


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

int handle_cd(int netfd, char *argument, char *cwd, int cwdsize);
int handle_ls(int netfd,char* argument,char* cwd);
int handle_puts(int netfd,char *arg);
int handle_gets(int netfd,char *filename,char* cwd);
int handle_remove(int netfd,char *arg);
int handle_pwd(int netfd,char *arg);
int handle_mkdir(int netfd,char *argument,char *cwd, int cwdsize);
int handle_rmdir(int netfd,char *argument,char *cwd, int cwdsize);
int handle_rm(int netfd,char* argument,char* cwd);
int handle_pwd(int netfd,char *cwd);
int uploadFile(int netfd, char* argument,char* cwd);
int recvn(int netfd, void *buf, int length);


#endif