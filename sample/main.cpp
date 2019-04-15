#include "rlog.h"
#include <pthread.h>

void* thread_main(void* arg)
{
    int id = *(int*)(arg);

    for (int i = 0; i < 1000000; ++i)
    {
        if (id == 1)
        {
            DEBUG_LOG("111111111111111111111111111111");
        }
        else if (id == 2)
        {
            DEBUG_LOG("2222222222222222222222222222222");
        }
        else if (id == 3)
        {
            DEBUG_LOG("3333333333333333333333333333333");
        }
    }
    return NULL;
}

int main(int argc, const char* argv[])
{
    rlog::InitGlobalLog("trace");

    pthread_t tid_1;
    int arg1 = 1;
    pthread_create(&tid_1, nullptr, thread_main, (void*)&arg1);
    pthread_t tid_2;
    int arg2 = 2;
    pthread_create(&tid_2, nullptr, thread_main, (void*)&arg2);
    pthread_t tid_3;
    int arg3 = 3;
    pthread_create(&tid_3, nullptr, thread_main, (void*)&arg3);

    pthread_join(tid_1, nullptr);
    pthread_join(tid_2, nullptr);
    pthread_join(tid_3, nullptr);
    return 0;
}
