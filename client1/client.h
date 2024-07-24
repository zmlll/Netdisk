#include <func.h>

typedef struct train_s
{
    int length;
    char data[1000];
}train_t;


typedef struct parsing_s
{
    char command[10];
    char argument[50];

}parsing_t;

int recvn(int sockfd,void* buf,int length);

int handle_cd(int sockfd);

parsing_t* commandparsing(parsing_t* parsing,char* buf);