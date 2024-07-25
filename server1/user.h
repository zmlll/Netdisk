#include <func.h>
#include <stdbool.h>

typedef struct User_s
{
    int num;
    char name[32];
    char key[9];
    char password[65];
    bool logout;
}User_t;

typedef struct parsing_s
{
    char command[10];
    char argument[50];

}parsing_t;

int user_create(User_t User[],int user_num,char* name,char* password);
int user_delete(User_t User[],int num,char* passwd);
int user_change(User_t User[],int num,char* name,char* passwd,char* new_passwd);
int user_login(User_t User[],int num,char* passwd);