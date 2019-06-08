#include <task.h>
#include <mutex.h>
#include <irq.h>
#include <tick.h>
#include <err.h>
#include <string.h>
#include <kdebug.h>

static int _mutex_lock(mutex_t *mutex, tick_t ticks)
{
    task_t *cur;
    irqstate_t state;
	tick_t now;
    int ret;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    /* mutex sholdn't wait in interrupt */
    if (is_in_interrupt() == true) {
        ret = ERR_IN_INTRPT;
        goto out;
    }

	cur = get_cur_task();

    /* mutex is available */
    if (mutex->cnt == 0) {
        mutex->cnt++;
        mutex->holder = cur;
        ret = ERR_OK;
        goto out;
    }

    /* mutex nest is allowed */
    if (mutex->holder == cur) {
        mutex->cnt++;
        ret = ERR_MUTEX_NESTED;
        goto out;
    }

    /* mutex is not available, the caller won't wait */
    if (ticks == WAIT_NONE) {
        ret = ERR_MUTEX_BUSY;
        goto out;
    }

    /* block current task */

    if (mutex->holder->prio > cur->prio) {
        /* boost up the holder's priority */
        task_set_prio(mutex->holder, cur->prio);
    }

	now = get_sys_tick();
	if (ticks != WAIT_FOREVER) {
		if (now + ticks < now) {
			leave_critical_section(state);
			return ERR_TICK_OVFLOW;
		}
		cur->pend_timeout = now + ticks;
	} else {
		cur->pend_timeout = WAIT_FOREVER;
	}

	cur->pend_list = &mutex->wait_list;
	cur->pend_ret_code = PEND_OK;
    task_list_add_prio(&mutex->wait_list, &cur->pend_node, cur->prio);
	task_ready_list_remove(cur);
    tick_list_insert(cur);
	cur->state = TS_PEND_MUTEX;
	ret = task_switch();
    leave_critical_section(state);

	if (ret != ERR_OK)
		return ret;

    /* so far, the task is restored */
    state = enter_critical_section();
	cur->pend_list = NULL;
	ret = task_pend_ret_code_convert(cur->pend_ret_code);
	if (ret == ERR_WAIT_TIMEOUT) {
		ret = ERR_MUTEX_TIMEOUT;
	}

out:
    leave_critical_section(state);

    return ret;
}

int mutex_init(mutex_t *mutex)
{
    KASSERT(mutex != NULL);

    memset(mutex, 0, sizeof(mutex_t));
    mutex->magic = MUTEX_MAGIC;
    mutex->cnt = 0;
    list_head_init(&mutex->wait_list);

    return ERR_OK;
}

int mutex_lock(mutex_t *mutex)
{
    return _mutex_lock(mutex, WAIT_FOREVER);
}

int mutex_timedlock(mutex_t *mutex, tick_t ticks)
{
    return _mutex_lock(mutex, ticks);
}

int mutex_trylock(mutex_t *mutex)
{
    return _mutex_lock(mutex, WAIT_NONE);
}

int mutex_unlock(mutex_t *mutex)
{
    irqstate_t state;
    task_t *cur;
    task_t *wakeup;
    int ret = ERR_OK;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

	if (mutex->holder == cur) {
		mutex->cnt--;
		if (mutex->cnt > 0) {
			goto out;
		}

		if (!list_is_empty(&mutex->wait_list)) {
	        wakeup = list_first_entry(&mutex->wait_list, task_t, pend_node);

			KASSERT(wakeup->state == TS_PEND_MUTEX
			        || wakeup->state == TS_PEND_MUTEX_SUSPEND);
			/* remove task from tick list */
	        list_delete(&wakeup->node);

			/* remove task from mutex wait list */
			list_delete(&wakeup->pend_node);

			mutex->cnt++;
	        mutex->holder = wakeup;
	        wakeup->pend_ret_code = PEND_WAKEUP;

	        task_become_ready(wakeup);

			if (cur->prio != cur->origin_prio) {
				cur->prio = cur->origin_prio;
				task_ready_list_remove(cur);
				task_become_ready(cur);
			}

			if (cur->prio > wakeup->prio) {
	        	ret = task_switch();
				goto out;
			}
		}else {
			mutex->holder = NULL;
		}
	} else {
		/* current task doesn't hold the mutex */
		ret = ERR_MUTEX_HOLDER;
		goto out;
	}

out:
    leave_critical_section(state);

    return ret;
}

