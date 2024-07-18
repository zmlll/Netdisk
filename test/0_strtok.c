#include <func.h>

int main(int argc,char* argv[])
{
    char str[] = "cd dir1";
    printf("%s\n",strtok(str," "));
    char *p;
    while((p = strtok(NULL," "))!=NULL)
    {
        printf("p = %s\n",p);
    }
    return 0;
}
