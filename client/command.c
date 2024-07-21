#include "client.h"

//命令行参数分割 
void stringsplit(char* buf,parsing_t parsing,char split)
{
    int flag = 0;
    for(int i =0;i<strlen(buf);i++)
    {
        if(flag == 0 && buf[i] == split)
        {
            strncpy(parsing.command,buf,i);
            flag =1;
        }
        else if(flag == 1&&(buf[i]== split||buf[i]=='\n' ))
        {
            strncpy(parsing.argument,buf+strlen(parsing.command)+1,i-(strlen(parsing.command)));
        }
    }
}

parsing_t* commandparsing(parsing_t* parsing,char* buf)
{
    //获取指令
    char* commandT = strtok(buf," ");
    //获取参数
    char* argumentT= strtok(NULL," ");
    
    strncpy(parsing->command,commandT,strlen(commandT));
    if(argumentT !=NULL)
    {
        strncpy(parsing->argument,argumentT,strlen(argumentT));
    }
    
    size_t len = strcspn(parsing->command,"\n"); //获取换行符的位置
    parsing->command[len] = '\0'; //将换行符替换为0

    if(argumentT!=NULL)
    {
        size_t len1 = strcspn(parsing->argument,"\n");
        parsing->argument[len1] = '\0';
    }

    return parsing;
}
