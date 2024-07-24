#include "threadPool.h"

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
        //kill -10 tid
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

    int sockfd;
    tcpInit(&sockfd,server_ip,server_port);
    int epfd = epoll_create(1);
    epollAdd(epfd,sockfd);
    //监听管道读端
    epollAdd(epfd,pipefds[0]);

    struct epoll_event readySet[2];
    while(1)
    {
        int readyNum = epoll_wait(epfd,readySet,2,-1);
        for(int i =0;i<readyNum;i++)
        {
            if(readySet[i].data.fd == sockfd)
            {
                struct sockaddr_in cliAddr;
                socklen_t cliAddrSize = sizeof(cliAddrSize);
                int netfd = accept(sockfd,(struct sockaddr*)&cliAddr,&cliAddrSize);
                printf("client ip = %s\n",inet_ntoa(cliAddr.sin_addr));
                printf("client port =%d\n", ntohs(cliAddr.sin_port));
                pthread_mutex_lock(&threadPool.mutex);
                //将netfd加入队列
                Enqueue(&threadPool.taskQueue,netfd);
                printf("main sent a new task!\n");

                pthread_cond_signal(&threadPool.cond);
                pthread_mutex_unlock(&threadPool.mutex);
            }
            else if(readySet[i].data.fd == pipefds[0])
            {
                //退出
                pthread_mutex_lock(&threadPool.mutex);
                threadPool.exitflag = 1;
                pthread_cond_broadcast(&threadPool.cond);
                pthread_mutex_unlock(&threadPool.mutex);

                for(int j =0;j<workerNum;j++)
                {
                    pthread_join(threadPool.tidArr[j],NULL);
                }
                printf("main thread is going to die!\n");

                pthread_exit(NULL);
            }

        }
    }



}