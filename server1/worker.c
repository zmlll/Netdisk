#include "threadPool.h"
#include "user.h"
// void *threadFunc(void* arg)
// {
//     threadPool_t* pthreadPool = (threadPool_t*)arg;
//     while(1)
//     {
//         //取出任务
//         pthread_mutex_lock(&pthreadPool->mutex);
//         while(pthreadPool->taskQueue.queueSize == 0)
//         {
//             pthread_cond_wait(&pthreadPool->cond,&pthreadPool->mutex);
//         }
//         printf("worker got a task!\n");
//         int netfd = pthreadPool->taskQueue.pfront->fd;
//         Dequeue(&pthreadPool->taskQueue);
//         pthread_mutex_unlock(&pthreadPool->mutex);
//         transFile(netfd);
//         close(netfd);
//     } 
// }

int makeWorker(threadPool_t* pthreadPool)
{
    for(int i = 0;i<pthreadPool->workerNum;i++)
    {
        pthread_create(&pthreadPool->tidArr[i],NULL,threadFunc,pthreadPool);
    }
    return 0;
}


void *threadFunc(void* arg)
{
    char cwd[2048] = "user";
    printf("threadFunc:cwd==%s\n",cwd);
    const char *CMD[9] = {
        "cd",
        "ls",
        "puts",
        "gets",
        "remove",
        "pwd",
        "mkdir",
        "rmdir",
        "clientExit"};

    threadPool_t *pthreadPool = (threadPool_t *)arg;
    User_t User[1024]; //user information
    int user_num = 0; //user number
    while(1)
    {
        int netfd;
        pthread_mutex_lock(&pthreadPool->mutex);

        // 退出标志 0 不退出 1 退出
        if (pthreadPool->exitflag == 1)
        {
            printf("I am going to die!\n");
            pthread_mutex_unlock(&pthreadPool->mutex);
            pthread_exit(NULL);
        }

        while(pthreadPool->taskQueue.queueSize == 0)
        {
            if(pthreadPool->exitflag == 1)
            {
                printf("I am going to die!\n");
                pthread_mutex_unlock(&pthreadPool->mutex);
                pthread_exit(NULL);
            }
           
           pthread_cond_wait(&pthreadPool->cond,&pthreadPool->mutex);
        }

        netfd = pthreadPool->taskQueue.pfront->fd;
        Dequeue(&pthreadPool->taskQueue);
        pthread_mutex_unlock(&pthreadPool->mutex);// 至此解锁，之后就可以执行任务了,执行任务不用放在临界区
        // 这句是非常关键的，要实行子线程连接客户端，从队列中取出的netfd是关键！！

        char buf[4096];
        bzero(buf,sizeof(buf));

        //the section of registration and login
        char registerOrLogin[4096];

Reinput:
        bzero(registerOrLogin,sizeof(registerOrLogin));
        sprintf(registerOrLogin,
                "\n---------------------------------------------------------\n"
                "-----------------------Options Page----------------------\n"
                "---------------------------------------------------------\n"
                "register     login     cancellation     changeYourPassword\n"
                "please input[r/l/cancel/change]\n(correspond to the above functions in order)\n");               
        send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
        bzero(registerOrLogin,sizeof(registerOrLogin));
        recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);

        int user_number = -1;
        char user_name[128];
        char password[128];
        char new_password[128];
        char cancel_confirm[128];
        int user_number_buf = -1;
        // bzero(user_name,sizeof(user_name));//在写入登录日志信息时，需要用到用户名，所以这里注释掉了，应该对整体没有影响
        bzero(password,sizeof(password));
        bzero(new_password,sizeof(new_password));

        if(registerOrLogin[0] == 'r')
        {
            bzero(registerOrLogin,sizeof(registerOrLogin));
            sprintf(registerOrLogin,
                   "\n---------------------------------------------------------\n"
                   "--------------------Registration Page--------------------\n"
                   "---------------------------------------------------------\n"
                   "Please input your user name:\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            strcpy(user_name,registerOrLogin);
            bzero(registerOrLogin,sizeof(registerOrLogin));

            sprintf(registerOrLogin,"\nPlease input your user password:\n");

            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            strcpy(password,registerOrLogin);

            int user_ret = user_create(User,user_num,user_name,password);
            if(user_ret == -1)
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,
                       "Registration faliure. \n"
                       "The user name must contain a maximum of 32characters,and the password must contain 8 to 16 characters,"
                       "recheck the input. \n"
                       "Please enter any letter to return to the options page.\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                
                goto Reinput;
            }
            else
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,
                       "Registration success.\n"
                       "Your user number is %d\n"
                       "Please enter any letter to return to the options page.\n",user_ret);
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                user_num++;   
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                goto Reinput;
            }
        }
        else if(registerOrLogin[0] == 'l')
        {
            bzero(registerOrLogin,sizeof(registerOrLogin));
            bzero(password,sizeof(password));
            sprintf(registerOrLogin,
                   "\n---------------------------------------------------------\n"
                   "-----------------------Login Page------------------------\n"
                   "---------------------------------------------------------\n"
                   "Please input your user number:\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            bzero(registerOrLogin,sizeof(registerOrLogin));
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            user_number = (int)registerOrLogin[0] - 48;

            bzero(registerOrLogin,sizeof(registerOrLogin));
            sprintf(registerOrLogin,"\nPlease input your user password:\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            strcpy(password,registerOrLogin);

            int login_ret = user_login(User,user_number,password);
            if(login_ret == -1)
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"Login faliure.\n"
                       "Please recheck your input.\n"
                    "Please enter any letter to return to the options page.\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);

                goto Reinput;
            }
            else
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"Login success.\n"
                "Please enter any letter to continue.\n");
                //登录成功，将用户登录信息写入日志
                LOG_LOGIN(user_name,clientAddr);
                printf("client ip = %s\n", inet_ntoa(clientAddr.sin_addr));
                printf("client port =%d\n", ntohs(clientAddr.sin_port));


                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"Out of system");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            }
            //Enter the login page
            //if the login fails,login again(prompt).
        }
         else if(strcmp(registerOrLogin,"cancel") == 0)
        {
confirm:
            bzero(registerOrLogin,sizeof(registerOrLogin));
            sprintf(registerOrLogin,
                   "\n---------------------------------------------------------\n"
                   "--------------------Cancellation Page--------------------\n"
                   "---------------------------------------------------------\n"
                   "Whether you want to close your account? [y/n]\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            bzero(registerOrLogin,sizeof(registerOrLogin));
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            strcpy(cancel_confirm,registerOrLogin);

            if(strlen(cancel_confirm) != 1)
            {
                bzero(cancel_confirm,sizeof(cancel_confirm));
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"\nPlease input ONLY ONE character.\n"
                        "Please enter any letter to return to the options page.\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);

                goto confirm;
            }
            if(cancel_confirm[0] == 'y' || cancel_confirm[0] == 'Y')
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"\nplease input your user number\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                user_number = (int)registerOrLogin[0] - 48;
                //user_number = registerOrLogin[0];
                
                bzero(registerOrLogin,sizeof(registerOrLogin));
                bzero(password,sizeof(password));
                sprintf(registerOrLogin,"\nPlease input your user password:\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                strcpy(password,registerOrLogin); 

                int delete_ret = user_delete(User,user_number,password);
                if(delete_ret == -1)
                {

                    bzero(registerOrLogin,sizeof(registerOrLogin));
                    sprintf(registerOrLogin,"Cancellation faliure.\n"
                           "Recheck your input.\n"
                    "Please enter any letter to return to the cancellation page.\n");
                    send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                    recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                    goto confirm;
                }
                else
                {
                    bzero(registerOrLogin,sizeof(registerOrLogin));
                    sprintf(registerOrLogin,"Your account has been cancellation.\n"
                    "Please enter any letter to return to the options page.\n");
                    send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                    recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);

                    goto Reinput;
                }
            }
            else if(cancel_confirm[0] == 'n' || cancel_confirm[0] == 'N')
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"The cancellation application was withdrawn.\n"
                        "Please enter any letter to return to the options page.\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                goto Reinput;
            }
            else
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"Cancellation faliure.\n"
                       "Please recheck your input.\n"
                       "Please enter any letter to return to the cancellation page.\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                goto confirm;
            }
                //cancellation
        }
        else if(strcmp(registerOrLogin,"change") == 0)
        {
            bzero(registerOrLogin,sizeof(registerOrLogin));
            sprintf(registerOrLogin,
                   "\n------------------------------------------------------------\n"
                   "--------------------Password Change Page--------------------\n"
                   "------------------------------------------------------------\n"
                   "\nplease input your user name:\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);

            bzero(registerOrLogin,sizeof(registerOrLogin));
            bzero(user_name,sizeof(user_name));
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            strcpy(user_name,registerOrLogin);

            bzero(registerOrLogin,sizeof(registerOrLogin));
            sprintf(registerOrLogin,"\nplease input your user number\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            user_number = (int)registerOrLogin[0] - 48;
            //user_number = registerOrLogin[0];

            bzero(registerOrLogin,sizeof(registerOrLogin));
            bzero(password,sizeof(password));
            sprintf(registerOrLogin,"\nPlease input your user password:\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            strcpy(password,registerOrLogin);

            bzero(registerOrLogin,sizeof(registerOrLogin));
            bzero(new_password,sizeof(new_password));
            sprintf(registerOrLogin,"\nPlease input your new user password:\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            strcpy(new_password,registerOrLogin);
            
            int change_ret = user_change(User,user_number,user_name,password,new_password);
            if(change_ret == -1)
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"Your password changed failure.\n"
                       "Please recheck your input.\n"
                "Please enter any letter to return to the options page.\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);

                goto Reinput; 
            }
            else
            {
                bzero(registerOrLogin,sizeof(registerOrLogin));
                sprintf(registerOrLogin,"Your password changed success.\n"
                        "Please enter any letter to return to the options page.\n");
                send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
                recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);

                goto Reinput;
            }
            //changePassword
        }
        else
        {
            bzero(registerOrLogin,sizeof(registerOrLogin));
            sprintf(registerOrLogin,"Input error, please enter,Please enter any letter to return to the options page.\n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);
            recv(netfd,registerOrLogin,sizeof(registerOrLogin),0);

            goto Reinput;
        }

        //Out of system
            bzero(registerOrLogin,sizeof(registerOrLogin));
            sprintf(registerOrLogin,
                   "\n------------------------------------------------------------\n"
                   "------------------------Command Page------------------------\n"
                   "------------------------------------------------------------\n"
                   "\nplease input your command: \n");
            send(netfd,registerOrLogin,sizeof(registerOrLogin),0);

        while(1)
        {
            parsing_t parsing;

            char command[10];
            char argument[50];
            memset(&parsing,0,sizeof(parsing));
           // 取出netfd指向的文件对象中的接收缓冲区域的数据
           // 便可以知道客户端向服务端的子线程发送了什么命令
           int ret = recv(netfd,&parsing,sizeof(parsing_t),0);
            if(ret == 0)
            {
                break;
            }

            strcpy(command,parsing.command);
            strcpy(argument,parsing.argument);
            LOG_FUNC(user_name,command,argument);

            if(strncmp(CMD[8],command,strlen(CMD[8]))==0)
            {
                printf("exit\n");
                break;
            }
            int cmdtoInt = -1;
            for(int i = 0;i<8;i++)
            {
                if(strncmp(CMD[i],command,strlen(CMD[8]))==0)
                {
                    cmdtoInt = i;
                    break;
                }
            }
            if(cmdtoInt == -1)
            {
                cmdtoInt = 8;
            }

            int i = 0;
            switch(cmdtoInt)
            {
                case CD:
                    i = 0;
                    send(netfd,&i,sizeof(int),0);
                    handle_cd(netfd,argument,cwd,sizeof(cwd));
                    break;
                case LS:
                    i = 1;
                    send(netfd,&i,sizeof(int),0);
                    handle_ls(netfd,argument,cwd);
                    break;
                case PUTS:
                    i = 2;
                    send(netfd,&i,sizeof(int),0);
                    uploadFile(netfd,argument,cwd);
                    break;
                case GETS:
                    i =3;
                    send(netfd,&i,sizeof(int),0);
                    handle_gets(netfd,argument,cwd);
                    break;
                case REMOVE:
                    i = 4;
                    send(netfd,&i,sizeof(int),0);
                    handle_rm(netfd,argument,cwd);
                    break;
                case PWD:
                    int i =5;
                    send(netfd,&i,sizeof(int),0);
                    handle_pwd(netfd,cwd);
                case MKDIR:
                    i = 6;
                    send(netfd,&i,sizeof(int),0);
                    handle_mkdir(netfd, argument,cwd,sizeof(cwd));
                    break;
                case RMDIR:
                    i = 7;
                    send(netfd,&i,sizeof(int),0);
                    handle_mkdir(netfd,argument,cwd,0);
                    break;
                case INVALID:
                    i = 8;
                    send(netfd,&i,sizeof(int),0);
                    break;
                default:
                    break;
            }
            printf("child thread finished task!\n");
        }
        close(netfd);
    }

}