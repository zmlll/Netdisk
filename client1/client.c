#include "client.h"

int main(int argc,char* argv[])
{
    // ./client 0.0.0.0 1234
    ARGS_CHECK(argc,3);
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(atoi(argv[2]));
    serAddr.sin_addr.s_addr = inet_addr(argv[1]);
    int ret = connect(sockfd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"connect");

    //客户端也得监听stdin和sockfd
    int epfd = epoll_create(1);
    epollAdd(epfd,sockfd);
    epollAdd(epfd,STDIN_FILENO);

    char buf[4096];

    //用户注册 登录系统
    while(1)
    {
        recv(sockfd,buf,sizeof(buf),0);
        if(strcmp(buf,"Out of system")==0)
        {
            break;
        }
        printf("%s\n",buf);
        scanf("%s",buf);
        send(sockfd,buf,sizeof(buf),0);
    }

    //退出 系统
    recv(sockfd,buf,sizeof(buf),0);
    printf("%s\n",buf);
    bzero(buf,sizeof(buf));

    struct epoll_event readySet[2];
    parsing_t parsing;
    while (1)
    {
        int readyNum = epoll_wait(epfd,readySet,2,-1);
        for(int i = 0;i<readyNum;i++)
        {
            if(readySet[i].data.fd == sockfd)
            {
                int j;
                recvn(sockfd,&j,sizeof(j));
                train_t train;
                bzero(&train,sizeof(train_t));
                int ret = 0;
                char temp[4096];
                switch(j)
                {
                    case 0:
                        handle_cd(sockfd);
                        break;
                    case 1:
                        bzero(temp,sizeof(temp));
                        read(sockfd,temp,sizeof(temp));
                        printf("%s\n",temp);
                        break;
                    case 2:
                        //将本地文件上传至服务器
                        handle_puts(sockfd,parsing.argument);
                        break;
                    case 3:
                        //客户端sockfd与服务端netfd通信，由子线程获得netfd发送文件
                        recvFile(sockfd);
                        break;
                    case 4:
                        printf("remove successfully\n");
                        break;
                    case 5:
                        //pwd 显示目前所在路径
                        bzero(buf,sizeof(buf));
                        recv(sockfd,buf,sizeof(buf),0);
                        printf("current working directory = %s\n",buf);
                        break;
                    case 6:
                        recv(sockfd,&ret,sizeof(int),0);
                        ret == 0?printf("创建目录执行成功返回:%d\n",ret):printf("创建目录执行失败:%d\n",ret);
                        break;
                    case 7:
                        recv(sockfd,&ret,sizeof(int),0);
                        ret == 0 ? printf("删除目录执行成功返回:%d\n", ret) : printf("删除目录执行失败:%d\n", ret);
                        break; 
                    case 8:
                        break;
                    default:
                        break;
                }
            }
            else
            {
                //检测到客户端用户输入指令如cd ls
                memset(buf,0,sizeof(buf));
                ssize_t sret = read(STDIN_FILENO,buf,sizeof(buf));
                if(sret == 0)
                {
                    send(sockfd,"clientExit",10,0);
                    goto end;
                }

                memset(&parsing,0,sizeof(parsing_t));
                commandparsing(&parsing,buf);

                if(strlen(parsing.command)!=0)
                {
                    //将客户端输入的命令通过sockfd发送到服务端
                    send(sockfd,&parsing,sizeof(parsing_t),0);
                }

            }
        }

    }

end:
    sleep(3);
    return 0;
}