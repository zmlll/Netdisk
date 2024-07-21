#include "client.h"

enum cd_retval
{
    CD_SUCCESS,
    ARGS_ILLEGAL,
    NO_SUCH_DIR
};

int handle_cd(int netfd)
{
    enum cd_retval cdret;
    int rdret = read(netfd,&cdret,sizeof(cdret));
    ERROR_CHECK(rdret,-1,"cd_client_read");
    if(cdret == CD_SUCCESS)
    {
        printf("cd_success!\n");
    }
    else if(cdret == ARGS_ILLEGAL)
    {
        printf("command args is illegal!\n");
    }
    else
    {
        printf("no such dir!\n");
    }
    return 0;
}