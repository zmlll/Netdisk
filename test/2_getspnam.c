#include <func.h>
#include <shadow.h>
int main(int argc, char *argv[])
{
    struct spwd * psw = getspnam("liao");
    printf("name = %s, encrypted password = %s\n",
           psw->sp_namp , psw->sp_pwdp);
    return 0;
}