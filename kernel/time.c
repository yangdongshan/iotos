#include <time.h>
#include <tick.h>
#include <task.h>
#include <core.h>
#include <err.h>
#include <kdebug.h>

unsigned long g_sys_ticks = 0;

/**
 * @brief let task sleep ms
 * @param[in] ms the time to sleep
 *
 * @return On success, return 0, if the task is iterrputed by siganl
 *          or cancelled, return negative error code
 */
int msleep(unsigned int ms)
{
    irqstate_t state;
    tick_t cur_tick;
    int ret = -1;

    state = enter_critical_section();
    task_t *task = get_cur_task();

    KDBG("task %s go to sleep\r\n", task->name);

    register_oneshot_timer(&task->wait_timer, task->name, MS2TICKS(ms), (timeout_cb)task_become_ready, task);

    task->state = TASK_SLEEPING;
    task->pend_ret_code = TASK_PEND_NONE;
    task_switch();
    leave_critical_section(state);

    state = enter_critical_section();
    if (task->pend_ret_code == TASK_PEND_TIMEOUT)
        ret = 0;
    else if (task->pend_ret_code == TASK_PEND_WAKEUP) {
        cur_tick = get_sys_tick();
        ret = task->wait_timer.timeout - cur_tick;
    } else {
        ret = -(task->pend_ret_code);
    }
    leave_critical_section(state);

    return ret;
}

