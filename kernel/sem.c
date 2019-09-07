#include <typedef.h>
#include <irq.h>
#include <sem.h>
#include <task.h>
#include <list.h>
#include <mm.h>
#include <tick.h>
#include <err.h>
#include <kdebug.h>
#include <string.h>

int sem_init(sem_t *sem, int val)
{
    int ret = SEM_OK;

    KASSERT(sem != NULL);

    memset(sem, 0, sizeof(sem_t));
    sem->magic = SEM_MAGIC;
    sem->cnt = val;
    list_head_init(&sem->wait_list);

    return ret;
}

static int _sem_wait(sem_t *sem, tick_t ticks)
{
    task_t *cur;
    irqstate_t state;
	tick_t now;
    int ret;

    KASSERT(sem != NULL);
    KASSERT(sem->magic == SEM_MAGIC);

    state = enter_critical_section();

    /* sem sholdn't wait in interrupt */
    if (is_in_interrupt() == true) {
        ret = -ERR_IN_INTRPT;
        goto out;
    }

	if (sem->cnt > 0) {
        sem->cnt--;
		ret = ERR_OK;
		goto out;
	}

	if (sem->cnt + SEM_CNT_THRESHOLD < 0) {
		ret = ERR_SEM_OVERFLOW;
		goto out;
	}

	/* mutex is not available, the caller won't wait */
    if (ticks == WAIT_NONE) {
        ret = ERR_SEM_BUSY;
        goto out;
    }


	cur = get_cur_task();
	now = get_sys_tick();
	if (ticks != WAIT_FOREVER) {
		if (now + ticks < now) {
			ret = ERR_TICK_OVFLOW;
            goto out;
		}
		cur->pend_timeout = now + ticks;
	} else {
		cur->pend_timeout = WAIT_FOREVER;
	}

    sem->cnt--;

	cur->pend_list = &sem->wait_list;
	cur->pend_ret_code = PEND_OK;
    task_list_add_prio(&sem->wait_list, &cur->pend_node, cur->prio);
	task_ready_list_remove(cur);
    tick_list_insert(cur);
	cur->state = TS_PEND_SEM;
	ret = task_switch();
    leave_critical_section(state);

	if (ret != ERR_OK)
		return ret;

    /* so far, the task is restored */
    state = enter_critical_section();
	cur->pend_list = NULL;
	ret = task_pend_ret_code_convert(cur->pend_ret_code);
    if (ret == ERR_WAIT_TIMEOUT) {
        ret = ERR_SEM_TIMEOUT;
    }

out:
	leave_critical_section(state);

	return ret;
}

int sem_wait(sem_t *sem)
{
	return _sem_wait(sem, WAIT_FOREVER);
}


/** If wait timeout, return SEM_TIMEOUT
 *  else return SEM_OK
 */
int sem_timedwait(sem_t *sem, tick_t ticks)
{
	return _sem_wait(sem, ticks);
}

int sem_trywait(sem_t *sem)
{
	return _sem_wait(sem, WAIT_NONE);
}

int sem_post(sem_t *sem)
{
	irqstate_t state;
	task_t *cur;
	task_t *wakeup;
	int ret = ERR_OK;

	KASSERT(sem != NULL);
	KASSERT(sem->magic == SEM_MAGIC);

	state = enter_critical_section();

	if (sem->cnt >= SEM_CNT_THRESHOLD) {
		ret = ERR_SEM_OVERFLOW;
		goto out;
	}

	sem->cnt++;

	if (sem->cnt <= 0) {
		KASSERT(!list_is_empty(&sem->wait_list));
		wakeup = list_first_entry(&sem->wait_list, task_t, pend_node);

		KASSERT(wakeup->state == TS_PEND_SEM || wakeup->state == TS_PEND_SEM_SUSPEND);
		/* remove task from tick list */
		list_delete(&wakeup->node);

		/* remove task from mutex wait list */
		list_delete(&wakeup->pend_node);

		wakeup->pend_ret_code = PEND_WAKEUP;

		task_become_ready(wakeup);

		cur = get_cur_task();

		if (cur->prio > wakeup->prio) {
			ret = task_switch();
			goto out;
		}
	}

out:
	leave_critical_section(state);

	return ret;

}

