#include <time.h>
#include <timer.h>
#include <task.h>
#include <irq.h>
#include <err.h>
#include <kdebug.h>

unsigned long g_sys_ticks = 0;


int msleep(unsigned int ms)
{
    irqstate_t state;

    state = enter_critical_section();
    task_t *task = get_cur_task();

    KDBG("task %s go to sleep\r\n", task->name);

    register_oneshot_timer(&task->wait_timer, task->name, ms, (timeout_cb)task_become_ready_head, task);

    task->state = TASK_SLEEPING;
    task_switch();

    leave_critical_section(state);

    return NO_ERR;
}

