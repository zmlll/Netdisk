#include <func.h>
int main(int argc, char *argv[])
{
    char passwd[] = "123";
    char salt[] = "$1$6vJYq1.K$";
    printf("encryptPasswd = %s\n", crypt(passwd,salt));
    return 0;
}
