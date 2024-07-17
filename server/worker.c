#include "head.h"

int makeWorker(int workerNum,workerdata_t* workerdataArr)
{
    pid_t pid;
    int fds[2];
    for(int i =0;i<workerNum;++i)
    {
        socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
        pid = fork();
        if(pid == 0)
        {
            close(fds[0]);
            eventLoop(fds[1]);
        }
        close(fds[1]);
        workerdataArr[i].pid = pid;
        workerdataArr[i].status = FREE;
        workerdataArr[i].socketfd = fds[0];
        printf("worker %d ,pid = %d,socketfd = %d\n",i,pid,fds[0]);
    }
    return 0;
}


int eventLoop(int sockfd)
{
    while(1)
    {
        int netfd;
        int exitflag;
        recvfd(sockfd,&netfd,&exitflag);
        if(exitflag == 1)
        {
            printf("child is going to exit!\n");
            exit(0);
        }
        //...
        printf("child begins working!\n");
        //...
        close(netfd);
        printf("child finishes his job!\n");
        pid_t pid = getpid();
        write(sockfd,&pid,sizeof(pid));
    }
}
