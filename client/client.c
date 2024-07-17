#include <func.h>

int main(int argc,char* argv[])
{
    //./client ip port
    ARGS_CHECK(argc,3);
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    int ret = connect(sockfd,(struct sockaddr*)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"cpnnect");
    //...
    close(sockfd);
    return 0;
}
