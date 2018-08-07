#include <kernel.h>
#include <kdebug.h>

int thread_main(void *arg)
{
    int id = (int)arg;
    int cnt = 0;
    while (1) {
        kdebug_print("am thread %d, cnt %d\r\n", id, cnt++);
        msleep(1800);
    }
}


void thread_test()
{
    int thid[10];
    int i = 0;

    thid[i] = thread_create("thread0", 10, thread_main, (void*)i, 1024, 5, 0);
    thread_resume(thid[i]);
/*    i++;
    thid[i] = thread_create("thread1", 10, thread_main, (void*)i, 1024, 5, 0);
    thread_resume(thid[i]);
    i++;
    thid[i] = thread_create("thread2", 10, thread_main, (void*)i, 1024, 5, 0);
    thread_resume(thid[i]);
    i++;
    thid[i] = thread_create("thread3", 10, thread_main, (void*)i, 1024, 5, 0);
    thread_resume(thid[i]);
*/
}
