#include <func.h>
int main(int argc,char* argv[])
{
    printf("line = %d\n",__LINE__);
    printf("function = %s\n",__FUNCTION__);
    printf("file = %s\n",__FILE__);
    return 0;
}