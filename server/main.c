#include "head.h"
int exitpipe[2];
void sigFunc(int signum)
{
    printf("process pool is going to exit!\n");
    write(exitpipe[1],"1",1);
}

int main(int argc,char* argv[])
{
    //./ftpserver ../conf/server.conf
    ARGS_CHECK(argc,4);
    int workerNum = atoi(argv[3]);
    workerdata_t* workerdataArr = (workerdata_t*)calloc(workerNum,sizeof(workerdata_t));
    makeWorker(workerNum,workerdataArr);
    pipe(exitpipe);
    signal(SIGUSR1,sigFunc);
    int sockfd;
    tcpInit(&sockfd,argv[1],argv[2]);
    int epfd = epoll_create(1);
    epollAdd(epfd,sockfd);
    epollAdd(epfd,exitpipe[0]);
    for(int i = 0;i<workerNum;i++)
    {
        epollAdd(epfd,workerdataArr[i].socketfd);
    }
    int readyMax = workerNum+1;
    while(1)
    {
        struct epoll_event *readySet = (struct epoll_event*)calloc(readyMax,sizeof(struct epoll_event));
        int readyNum = epoll_wait(epfd,readySet,readyMax,-1);
        for(int i = 0;i<readyNum;i++)
        {
            if(readySet[i].data.fd == sockfd)
            {
                printf("master,client connect\n");
                int netfd = accept(sockfd,NULL,NULL);
                for(int j = 0;j<workerNum;j++)
                {
                    if(workerdataArr[j].status == FREE)
                    {
                        printf("No %d worker got his job,pid = %d\n",j,workerdataArr[j].pid);
                        sendfd(workerdataArr[i].socketfd,netfd,0);
                        workerdataArr[j].status = BUSY;
                        break;
                    }
                }
                close(netfd);
            }
            else if(readySet[i].data.fd == exitpipe[0])
            {
                for(int j = 0;j<workerNum;++j)
                {
                    sendfd(workerdataArr[j].socketfd,0,1);
                    printf("kill 1 worker!\n");
                }
                for(int j = 0;j<workerNum;++j)
                {
                    wait(NULL);
                }
                printf("master is going to exit!\n");
                exit(0);
            }
            else
            {
                printf("master, 1 child finishes his job!\n");
                for(int j =0;j<workerNum;j++)
                {
                    if(workerdataArr[j].socketfd == readySet[i].data.fd)
                    {
                        pid_t pid;
                        read(workerdataArr[j].socketfd,&pid,sizeof(pid));
                        printf("No %d worker,pid = %d\n",j,pid);
                        workerdataArr[j].status = FREE;
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
