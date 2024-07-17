#include <func.h>

enum
{
    FREE,
    BUSY 
};

typedef struct workerdata_s
{
    int status;
    pid_t pid;
    int socketfd;

}workerdata_t;

int makeWorker(int workerNum,workerdata_t* workerdataArr);
int eventLoop(int sockfd);
int tcpInit(int* psockfd,const char* ip,const char* port);
int sendfd(int sockfd,int fdtosend,int exitflag);
int recvfd(int sockfd,int *pfdtorecv,int* pexitflag);
int epollAdd(int epfd,int fd);
int epollDel(int epfd,int fd);
int transFile(int netfd);
