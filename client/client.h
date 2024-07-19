#include <func.h>

typedef struct train_s
{
    int length;
    char data[4096];
}train_t;


typedef struct parsing_s
{
    char command[10];
    char argument[50];
}parsing_t;


int epollAdd(int epfd,int fd);
int epollDel(int epfd,int fd);

int recvn(int sockfd,void* buf,int length);
int recvFile(int sockfd);

int handle_puts(int sockfd,char* argument);

int handle_cd(int sockfd);
int handle_ls(int sockfd);

parsing_t* commandparsing(parsing_t* parsing,char* buf);
