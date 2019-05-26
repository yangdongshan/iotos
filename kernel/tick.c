#include <tick.h>
#include <task.h>
#include <sched.h>
#include <kdebug.h>
#include <irq.h>
#include <err.h>
#include <list.h>
#include <string.h>

static list_head_t tick_list;

tick_t g_sys_ticks = 0;

void tick_init_early(void)
{
    list_head_init(&tick_list);
}

void tick_list_insert(task_t *task)
{
    task_t *iter;
    int inserted = 0;

    if (list_is_empty(&tick_list)) {
        list_add_head(&tick_list, &task->node);
    } else {
        list_foreach_entry(&tick_list, iter, task_t, node) {
            if (task->pend_timeout < iter->pend_timeout) {
                list_add_before(&iter->node, &task->node);
                inserted = 1;
                break;
            }
        }

        if (inserted == 0) {
            list_add_tail(&tick_list, &task->node);
        }
    }
}

int tick_cancel(ticker_t *ticker)
{
    ticker_t *iter;
    int ret = ERR_OK;

    KASSERT(ticker != NULL);
    KASSERT(!list_is_empty(&tick_list));
    KASSERT(!list_in_list(&ticker->node));

    list_foreach_entry(&tick_list, iter, ticker_t, node) {
        if (ticker == iter) {
            list_delete(&iter->node);
            ret = ERR_OK;
            break;
        }
    }

    return ret;
}

void tick_update(void)
{
    irqstate_t state;
    task_t *cur;
    task_t *task;
    tick_t now;

    if (SCHED_RUNNING != sched_get_state())
        return;

	KDBG("cur %s\r\n", get_cur_task()->name);
	
    state = enter_critical_section();

    now = get_sys_tick();
    while (!list_is_empty(&tick_list)) {
        task = list_first_entry(&tick_list, task_t, node);
        if (task->pend_timeout <= now) {
            KDBG("cur_tick 0x%llx, timeout tick 0x%llx\r\n",
                 now, task->pend_timeout);
            task->pend_ret_code = PEND_TIMEOUT;
            list_delete(&task->node);
			task_become_ready(task);
        } else {
            break;
        }
    }

    leave_critical_section(state);
}

