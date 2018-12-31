#include <task.h>
#include <sched.h>
#include <timer.h>
#include <mm.h>
#include <sem.h>
#include <workqueue.h>
#include <idle.h>

void os_init(void)
{
    os_set_state(OS_INIT);

    mm_init_early();

    timer_init_early();

    task_init_early();

    workqueue_init_early();

    create_idle_task();

    create_workqueue_task();

    os_set_state(OS_READY);
}

void os_run(void)
{
    os_start_sched();
}
