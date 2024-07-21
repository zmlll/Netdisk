#include <stdio.h>
#define OK 1
#define ERROR 0
#define TRUE 1
#define FALSE 0
#define MAXSIZE 20

typedef int ElemType;
typedef int State;
typedef struct 
{
    ElemType data[MAXSIZE];
    int top;
}SqStack;

State initStack(SqStack* S)
{
    S->top  = -1;
    return OK;
}

int getLength(SqStack S)
{
    return (S.top)+1;
}

State clearStack(SqStack* S)
{
    S->top = -1;
    return OK;
}

State isEmpty(SqStack* S)
{
    if (S->top == -1)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}


State push(SqStack* S,ElemType *e)
{
    if(S->top == MAXSIZE-1)
    {
        printf("栈满,无法入栈\n");
        return ERROR;
    }
    else
    {
        S->top++;
        S->data[S->top] = e;
        return OK;
    }
}

State pop(SqStack *S, ElemType *e){
    if(S->top==-1){   
        printf("栈空，无法出栈\n");
        return ERROR;
    }else{   
        *e = S->data[S->top];   
        S->top--;   
        return OK;
    }
}

State getTop(SqStack S, ElemType *e){
    if(S.top==-1){   
        printf("栈空，无栈顶元素\n");
        return ERROR;
    }else{   
        *e = S.data[S.top];   
        return OK;
    }
}

State printStack(SqStack S){
    if(S.top==-1){   
        printf("栈空\n");
        return ERROR;
    }
    int i=0;   
    while(S.top!=-1){
        i++;  
        printf("栈顶向下第%d个元素为:%d\n", i, S.data[S.top]);  
        S.top--;   
    }
    return OK;
}

int main(int argc, const char * argv[]) {
    SqStack S;
    initStack(&S);
    printf("初始化后的线性栈的长度为：%d\n", getLength(S));
    printf("将1-5元素依次入栈可得：\n");
    for(int i=1;i<=5;i++){
        push(&S, i);
    }
    printStack(S);
    printf("此时顺序栈的长度为：%d\n", getLength(S));
    int e;
    pop(&S, &e);
    printf("出栈：%d\n", e);
    pop(&S, &e);
    printf("出栈：%d\n", e);
    printf("现在顺序栈的长度为：%d\n", getLength(S));
    getTop(S, &e);
    printf("获取栈顶元素：%d\n", e);
    printf("现在顺序栈的长度为：%d\n", getLength(S));
    printf("现在顺序栈的为：\n");
    printStack(S);
    clearStack(&S);
    printf("清空顺序栈后的栈为：\n");
    printStack(S);
    printf("长度为：%d", getLength(S));
    return 0;
}
