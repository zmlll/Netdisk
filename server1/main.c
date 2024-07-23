#include "threadPool.h"
#include "taskQueue.h"
#include <func.h>


int pipefds[2];

void sighandler(int signum)
{
    printf("signum = %d\n",signum);
    //随便write一个数据，
    //目的是触发子进程的pipefds[0](使管道读端就绪)，然后子进程需要退出线程池
    write(pipefds[1],"1",1);
}
int main(int argc,char* argv[])
{
    //./ftpserver ../conf/server.conf
    ARGS_CHECK(argc,2);
    FILE *pfile = fopen(argv[1],"r+");
    char server_ip[20] = {0};
    char server_port[10] = {0};
    char thread_num[10] = {0};
    fscanf(pfile,"%s %s %s",server_ip,server_port,thread_num);
    fclose(pfile);

    pipe(pipefds);
    if(fork()!=0)
    {
        //父线程
        close(pipefds[0]);
        signal(SIGUSR1,sighandler);
        wait(NULL);
        exit(0);
    }

    threadPool_t threadPool;
    //子线程的数量argv[3]
    int workerNum = atoi(thread_num);
    //初始化，给子线程分配空间，初始化mutex和cond
    threadPoolInit(&threadPool,workerNum);
    //创建子线程
    makeWorker(&threadPool);

}