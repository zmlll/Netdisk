#include "threadPool.h"
int tcpInit(int *psockfd,const char* ip,const char* port)
{
    *psockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in cliaddr;
    cliaddr.sin_addr.s_addr = inet_addr(ip);
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(atoi(port));
    int reuse =1;
    int ret = setsockopt(*psockfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    ERROR_CHECK(ret,-1,"setsockopt");
    listen(*psockfd,10);
    return 0;
}