#include "../client/client.h"

int recvn(int sockfd,void *buf,int length)
{
    int total = 0;
    int ret ;
    char* p = (char*)buf;
    while(total < length)
    {
        ret = recv(sockfd,p+total,length-total,0);
        total += ret;
    }
    return 0;
}

