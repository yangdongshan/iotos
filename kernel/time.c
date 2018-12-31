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
    tick_t cur_tick;
    int ret = -1;

    state = enter_critical_section();
    task_t *task = get_cur_task();

    KDBG("task %s go to sleep\r\n", task->name);

    register_oneshot_timer(&task->wait_timer, task->name, MS2TICKS(ms), (timeout_cb)task_become_ready_head, task);

    task->state = TASK_SLEEPING;
    task->pend_ret_code = PEND_NONE;
    task_switch();

    if (task->pend_ret_code == PEND_NONE)
        ret = 0;
    else if (task->pend_ret_code == PEND_WAKEUP) {
        cur_tick = get_sys_tick();
        ret = task->wait_timer.timeout - cur_tick;
    }
    leave_critical_section(state);

    return ret;
}

