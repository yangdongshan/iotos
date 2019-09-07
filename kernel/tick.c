#include <tick.h>
#include <task.h>
#include <mutex.h>
#include <sem.h>
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

void tick_update(void)
{
    irqstate_t state;
    task_t *cur;
    task_t *task;
	task_t *owner;
	task_t *first_pend_task;
    tick_t   now;
	mutex_t *mutex;
    sem_t   *sem;

    if (SCHED_RUNNING != sched_get_state())
        return;

    state = enter_critical_section();

    now = get_sys_tick();
    while (!list_is_empty(&tick_list)) {
        task = list_first_entry(&tick_list, task_t, node);
		if (task->pend_timeout > now) {
			break;
		}

        KDBG("cur_tick 0x%llx, %s timeout tick 0x%llx\r\n",
             now, task->name, task->pend_timeout);
		task->pend_ret_code = PEND_TIMEOUT;
		list_delete(&task->node);
		switch (task->state) {
			case TS_PEND_SLEEP:
				task_become_ready(task);
				break;
			case TS_PEND_MUTEX:
				mutex = container_of(task->pend_list, mutex_t, wait_list);
				first_pend_task = list_first_entry(task->pend_list, task_t, pend_node);
				list_delete(&task->pend_node);
				if (task == first_pend_task && !list_is_empty(task->pend_list)) {
					first_pend_task = list_first_entry(task->pend_list, task_t, pend_node);
				}
				if (mutex->holder->prio < mutex->holder->origin_prio
					&& mutex->holder->origin_prio > first_pend_task->prio) {
					task_set_prio(mutex->holder, first_pend_task->prio);
				}
				task_become_ready(task);
				break;
			case TS_PEND_MUTEX_SUSPEND:
				mutex = container_of(task->pend_list, mutex_t, wait_list);
				first_pend_task = list_first_entry(task->pend_list, task_t, pend_node);
				list_delete(&task->pend_node);
				if (task == first_pend_task && !list_is_empty(task->pend_list)) {
					first_pend_task = list_first_entry(task->pend_list, task_t, pend_node);
				}
				if (mutex->holder->prio < mutex->holder->origin_prio
					&& mutex->holder->origin_prio > first_pend_task->prio) {
					task_set_prio(mutex->holder, first_pend_task->prio);
				}
				task->state = TS_PEND_MUTEX_TIMEOUT_SUSPEND;
				task_addto_suspend_list(task);
				break;
            case TS_PEND_SEM:
                sem = container_of(task->pend_list, sem_t, wait_list);
                list_delete(&task->pend_node);
                sem->cnt++;
                task_become_ready(task);
                break;
			default:
				KASSERT(0);

		}

    }

out:
    leave_critical_section(state);
}

