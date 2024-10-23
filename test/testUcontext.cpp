#include <ucontext.h>
#include <iostream>

#define STASK_SIZE 8192

ucontext_t mainContext, funContext;

void fun(int arg1, int arg2)
{
    std::cout<<"arg1 = "<<arg1<<" arg2 = "<<arg2<<std::endl;
}


int main(int argc, char **argv)
{

    getcontext(&funContext);//获取当前的运行环境
    funContext.uc_stack.ss_sp = new char[STASK_SIZE];
    funContext.uc_stack.ss_size = STASK_SIZE;
    funContext.uc_stack.ss_flags = 0;

    //设置后继上下文 fun执行完成之后 再回到main
    funContext.uc_link = &mainContext;

    //创造新的上下文 传递函数和参数
    makecontext(&funContext, (void(*)(void))fun, 2, 10, 20);

    std::cout<<"before swapcontext"<<std::endl;
    //此时发生上下文的切换
    swapcontext(&mainContext, &funContext);
    std::cout<<"after swapcontext"<<std::endl;

    delete []funContext.uc_stack.ss_sp;
    return 0;
}