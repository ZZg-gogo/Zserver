#include "Lock.h"
#include "Logger.h"


namespace BASE
{


Semaphore::Semaphore(int num)
{
    if (sem_init(&sem_, 0, num))
    {
        LOG_INFO(LOG_ROOT)<<"Semaphore sem_init error ";
        exit(-1);
    }
    
}

Semaphore::~Semaphore()
{
    sem_destroy(&sem_);
}

void Semaphore::wait()
{
    if (sem_wait(&sem_))
    {
         LOG_INFO(LOG_ROOT)<<"Semaphore sem_wait error ";
        exit(-1);
    }
    
}
void Semaphore::notify()
{
    if (sem_post(&sem_))
    {
        LOG_INFO(LOG_ROOT)<<"Semaphore notify error ";
        exit(-1);
    }
    
}

int Semaphore::getValue()
{
    int count = 0;
    if (sem_getvalue(&sem_, &count))
    {
        LOG_INFO(LOG_ROOT)<<"Semaphore sem_getvalue error ";
        exit(-1);
    }
    
    return count;
}




}