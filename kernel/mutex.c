#include <task.h>
#include <mutex.h>
#include <irq.h>
#include <err.h>
#include <string.h>
#include <kdebug.h>

static void _mutex_timeout_cb(void *arg)
{
    task_t *task = (task_t*)arg;

    list_delete(&task->node);
    task_become_ready(task);
    KINFO("task %s mutex wait timeout\r\n",
            task->name);
    task->pend_ret_code = PEND_TIMEOUT;
}

static int _mutex_wait(mutex_t *mutex, unsigned long ticks)
{
    task_t *cur;
    irqstate_t state;
    int ret;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    /* mutex sholdn't wait in interrupt */
    if (is_in_interrupt() == AX_TRUE) {
        ret = ERR_IN_INT;
        goto out;
    }

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
    if (ticks == AX_WAIT_NONE) {
        ret = ERR_MUTEX_BUSY;
        goto out;
    }

    /* block current task */
    cur = get_cur_task();

    if (mutex->holder->priority > cur->priority) {
        /* boost up the holder's priority */
        task_set_priority(mutex->holder, cur->priority);
    }

    cur->pend_list = &mutex->wait_list;
    cur->state = TS_PEND_MUTEX;
    cur->pend_ret_code = TASK_PEND_NONE;
    task_list_add_priority(&mutex->wait_list, cur);

    if (ticks != AX_WAIT_FOREVER) {
        register_oneshot_timer(&cur->wait_timer, "mutex", ticks,
                               _mutex_timeout_cb, cur);
    }

    task_switch();

    leave_critical_section(state);

    /* so far, the task is restored */
    state = enter_critical_section();

    ret = cur->pend_ret_code;
    cur->pending_list = NULL;

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

int mutex_wait(mutex_t *mutex)
{
    return _mutex_wait(mutex, AX_WAIT_FOREVER);
}

int mutex_timedwait(mutex_t *mutex, tick_t ticks)
{
    return _mutex_wait(mutex, ticks);
}

int mutex_trywait(mutex_t *mutex)
{
    return _mutex_wait(mutex, AX_WAIT_NONE);
}

int mutex_post(mutex_t *mutex)
{
    irqstate_t state;
    task_t *cur;
    task_t *wakeup;
    int ret;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    if (!list_is_empty(&mutex->wait_list)) {
        wakeup = list_first_entry(&mutex->wait_list, task_t, node);
        list_delete(&wakeup->node);
        mutex->holder = wakeup;
        wakeup->pend_ret_code = TASK_PEND_WAKEUP;
        task_become_ready(wakeup);
        task_become_ready(cur);
        task_switch();
        ret = ERR_OK;
        goto out;
    }

    if (mutex->cnt >= 1) {
        if (mutex->holder == cur) {
            mutex->cnt--;
            if (mutex->cnt == 0) {
                mutex->holder = NULL;
            }
            ret = ERR_OK;
            goto out;
        } else {
            /* current task doesn't hold the mutex */
            ret = ERR_MUTEX_HOLDER;
            goto out;
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

