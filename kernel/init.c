#include <task.h>
#include <sched.h>
#include <tick.h>
#include <mm.h>
#include <sem.h>
#include <workqueue.h>
#include <idle.h>

void os_init(void)
{
    sched_set_state(SCHED_INIT);

    mm_init_early();

    tick_init_early();

    task_init_early();

    //workqueue_init_early();

    create_idle_task();

    //create_workqueue_task();

    sched_set_state(SCHED_READY);
}

void os_run(void)
{
    sched_start();
}

