#include "client.h"

int recvn(int sockfd,void* buf,int length)
{
    int total = 0;
    int ret;
    char* p = (char*)buf;
    while(total <length)
    {
        ret = recv(sockfd,p+total,length-total,0);       
    }
}

int epollAdd(int epfd, int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    return 0;
}
int epollDel(int epfd, int fd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
    return 0;
}


//cd命令实现
enum cd_retval
{
    CD_SUCCESS,
    ARGS_ILLEGAL,
    NO_SUCH_DIR
};

int handle_cd(int netfd)
{
    enum cd_retval cdret;
    int rdret = read(netfd,&rdret,sizeof(cdret));
    ERROR_CHECK(rdret,-1,"cd_client_read");
    if(cdret == CD_SUCCESS)
    {
        printf("cd_success\n");
    }
    else if(cdret == ARGS_ILLEGAL)
    {
        printf("command args is illegal\n");
    }
    else
    {
        printf("no such dir\n");
    }
    return 0;
}

int handle_ls(int sockfd)
{
    char buf[1024];
    recv(sockfd,buf,sizeof(buf),0);
    printf("%s",buf);
    return 0;
}

int handle_puts(int sockfd,char* argument)
{
    int fd = open(argument,O_RDWR);
    if(fd == -1)
    {
        printf("文件不存在!\n");
        int signal = -1;
        send(sockfd,&signal,sizeof(signal),0);
        return 0;
    }
    else
    {
        int signal = 0;
        send(sockfd,&signal,sizeof(signal),0);
    }
    train_t train;
    struct stat statbuf;
    fstat(fd,&statbuf);
    bzero(fd,&statbuf);
    bzero(&train,sizeof(train));
    train.length = sizeof(statbuf.st_size);

    memcpy(train.data,&statbuf.st_size,train.length);
    send(sockfd,&train,sizeof(train.length)+train.length,MSG_NOSIGNAL);
    printf("文件大小:%2ld字节\n",statbuf.st_size);

    if(statbuf.st_size >= 104857600) //2^20 *100 100M
    {
        //文件过大时使用mmap
        printf("文件大于100M,使用mmap\n");
        char *p = (char* )mmap(NULL,statbuf.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
        ERROR_CHECK(p,MAP_FAILED,"mmap");
        send(sockfd,p,statbuf.st_size,MSG_NOSIGNAL); // 一次性将整个文件的内容send过去
        munmap(p,statbuf.st_size);
        int signal = -1;
        recv(sockfd, &signal, sizeof(signal), 0);
        if (signal == 0){
            printf("上传成功!\n");        
        }
    }
    else
    {
        char *p = (char *)mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ERROR_CHECK(p, MAP_FAILED, "mmap");
        ///
    }

}