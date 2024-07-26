#include <func.h>
enum cd_retval
{
    CD_SUCCESS,
    ARGS_ILLEGAL,
    NO_SUCH_DIR
};
void go_to_usr_root_dir(char* cwd,int cwsize)
{
    int cnt = 0;
    int cur = 0;
    while(*(cwd + cur)!='\0')
    {
        if(*(cwd + cur)== '/')
        {
            break;
        }
        cur++;
    }
    if(*(cwd +cur) == '/')
    {
        memset(cwd+cur,0,cwsize-cur);
        //把第一个/后面的全部置0，剩余部分即为用户根目录
        //退出循环的位置字符为'\0'，说明当前就已经是根目录了
    }
}

bool is_legal_dest(char* token)
{
    //字符串出现非法字符的情况
    for(int i =0;token[i] != '\0';i++)
    {
        if(token[i] < ' ' ||token[i] == 127)
        {
            //token出现不可打印字符
            return false;
        }
        else if(token[i] == '\\' || token[i] == ':' || token[i] =='*' ||
        token[i] == '?' || token[i] == '\"' || token[i] == '<' ||
        token[i] == '>' || token[i] == '|')
        {
            return false;
        }
        else if(token[i] == '/' && token[i+1] == '/')
        {
            return false;
        }
    }
    return true;
}


int handle_cd(int netfd,char* argument,char* cwd,int cwdsize)
{
    enum cd_retval cdret;
    char dest[2048] = {0};
    int end = 0;  // 存储dest的长度，也是dest第一个'\0'的下标
    //获取当前工作目录current work directory
    end = strlen(cwd);
    strncpy(dest,cwd,end);
    // 参数合法性判断与获取目标文件夹
    char token[1024] = {0};
    int left = 0,right = 0;
    int token_num = 0;
    while(argument != NULL)
    {
        if((*argument+right) == '\0')
        {
            //字符串的结构
            if(left!=right)
            {
                //第一个token，且遇到字符串结尾
                strncpy(token,argument+left,right-left);
                token_num++;
            }
            break;
        }
        else if(*(argument+right) == ' '&& left == right)
        {
            right++,left = right;
        }
        else if((*argument+right) == ' ' && left !=right)
        {
            strncpy(token,argument+left,right-left);
            right++,left = right;
            token_num++;
        }
        else if(*(argument + right)!= ' ')
        {
            if(token_num == 0) //遇到第一个token
            {
                right++;
            }
            else
            {
                //遇到第二个token，不合法
                token_num++;
                break;
            }
        }
    }
    if(token_num == 0)
    {
        //cd后无其他参数，回到用户根目录
        go_to_usr_root_dir(cwd,cwdsize);
        cdret = CD_SUCCESS;
        int wret = write(netfd,&cdret,sizeof(enum cd_retval));
        ERROR_CHECK(wret,-1,"cd_server_go_to_root_dir_write");
    }
    else if(token_num >1)
    {
        //发现多个token，命令参数非法
        cdret = ARGS_ILLEGAL;
        int wret = write(netfd,&cdret,sizeof(enum cd_retval));
        ERROR_CHECK(wret,-1,"cd_server_args_illegal_write");
    }
    else
    {
        //token == 1,参数量合法
        //地址合法性判断
        if(!is_legal_dest(token))
        {
            //发现非法字符
            cdret = ARGS_ILLEGAL;
            int wret = write(netfd,&cdret,sizeof(enum cd_retval));
            ERROR_CHECK(wret,-1,"cd_server_args_contain_illegal_char");
        }
        else
        {
            //没有非法字符
            char *tmpt = token;
            int tmptsize = strlen(tmpt);
            if(tmpt[0] == '/')
            {
                go_to_usr_root_dir(dest,sizeof(dest));
                end = strlen(dest);
                tmpt++;
                tmptsize--;
            }
            int left =0;
            int right = 0;
            char tmpc[1024] ={0};
            while(right < tmptsize)
            {
                // 按'/'进行字符串切割，如果是正常路径，就加入dest后，
                // 如果是'.'则不改变dest;
                // 如果是'..'则删除dest存储路径的最后一个目录
                if(right == tmptsize)
                {
                    //走到字符串尾
                    if(left != right)
                    {
                        //最后一段路径
                        strncpy(tmpc,tmpt+left,right-left);
                        if(strcmp(tmpc,"..")==0)
                        {
                            for(--end;end >= 0 && dest[end]!='/';--end)
                            {
                                dest[end] = '\0';
                            }
                            if(end >=0)
                            {
                                dest[end] = '\0';
                            }
                            if(strlen(dest) == 0)
                            {
                                //回退太多导致逻辑上到达用户根目录上级，逻辑非法
                                cdret = ARGS_ILLEGAL;
                                int wret = write(netfd,&cdret,sizeof(enum cd_retval));
                                ERROR_CHECK(wret,-1,"cd_server_args_illegal_logic");
                                return -1;
                            }
                        }
                        else if(strcmp(tmpc,".")!=0)
                        {
                            dest[end++] = '/';
                            //把tmpt从left开始，right-left长度的字符段复制到dest尾
                            strncpy(dest + end,tmpt+left,right-left);
                            end += right-left;
                        }
                        memset(tmpc,0,sizeof(tmpc));
                    }
                    break;
                }
                else if(tmpt[right] == '/' && left != right)
                {
                    //right走到一个路径端的尾部
                    strncpy(tmpc,tmpt+left,right-left);
                    if(strcmp(tmpc,"..")==0)
                    {
                        for(--end;end>=0 && dest[end]!='/';--end)
                        {
                            dest[end] = '\0';
                        }
                        if(end>=0)
                        {
                            dest[end] = '\0';
                        }
                        if(end <= 0)
                        {
                            //回退太多导致逻辑上到达用户根目录上级，逻辑非法
                            cdret = ARGS_ILLEGAL;
                            int wret = write(netfd,&cdret,sizeof(enum cd_retval));
                            ERROR_CHECK(wret,-1,"cd_server_args_illegal_logic");
                            return -1;
                        }
                    }
                    else if(strcmp(tmpc,".")!=0)
                    {
                        dest[end++] = '/';
                        strncpy(dest+end,tmpt + left,right - left);
                        end += right -left;
                    }
                    right++,left = right;
                    memset(tmpc,0,sizeof(tmpc));
                }
                else
                {
                    right++;
                }
            }
            //判断是否存在要求路径
            char buf[2048] = {0};
            getcwd(buf,sizeof(buf));
            int bufsize = strlen(buf);
            buf[bufsize] = '/';
            bufsize++;
            strncpy(buf+bufsize,dest,strlen(dest));
            DIR* pdir = opendir(buf);
            if(pdir == NULL)
            {
                cdret = NO_SUCH_DIR;
                int wret = write(netfd,&cdret,sizeof(enum cd_retval));
                ERROR_CHECK(wret,-1,"cd_server_write_no_such_dir");
                return -1;
            }
            else
            {
                memset(cwd,0,cwdsize);
                strncpy(cwd,dest,strlen(dest));
                cdret = CD_SUCCESS;
                int wret = write(netfd,&cdret,sizeof(enum cd_retval));
                ERROR_CHECK(wret,-1,"cd_server_write_cd_success");
            }
        }
    }
    printf("%s\n",cwd);
    return 0;
}
