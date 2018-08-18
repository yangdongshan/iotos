#include <kdebug.h>

int task_main(void *arg)
{
    int id = (int)arg;
    int cnt = 0;
    while (1) {
        kdebug_print("am task %d, cnt %d\r\n", id, cnt++);
        msleep(1800);
    }
}


void task_test()
{
    int thid[10];
    int i = 0;

    //thid[i] = task_create("task0", 10, task_main, (void*)i, 1024, 5, 0);
    //task_resume(thid[i]);
/*    i++;
    thid[i] = task_create("task1", 10, task_main, (void*)i, 1024, 5, 0);
    task_resume(thid[i]);
    i++;
    thid[i] = task_create("task2", 10, task_main, (void*)i, 1024, 5, 0);
    task_resume(thid[i]);
    i++;
    thid[i] = task_create("task3", 10, task_main, (void*)i, 1024, 5, 0);
    task_resume(thid[i]);
*/
}
