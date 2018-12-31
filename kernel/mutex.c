#include <task.h>
#include <mutex.h>
#include <irq.h>
#include <kdebug.h>

int mutex_init(mutex_t *mutex)
{
    KASSERT(mutex != NULL);

    memset(mutex, 0, sizeof(mutex_t));
    mutex->magic = MUTEX_MAGIC;
    mutex->cnt = 0;
    list_head_init(&mutex->wait_list);

    return MUTEX_OK;
}

int mutex_wait(mutex_t *mutex)
{
    task_t *cur;
    irqstate_t state;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    if (mutex->cnt == 0) {
        mutex->cnt++;
        mutex->holder = cur;
        leave_critical_section(state);
        return MUTEX_OK;
    }

    if (mutex->holder == cur) {
        mutex->cnt++;
        leave_critical_section(state);
        return MUTEX_ENESTED;
    }

    if (mutex->holder->priority > cur->priority) {
        task_set_priority(mutex->holder, cur->priority);
    }

    cur->pending_list = &mutex->wait_list;
    cur->state = TASK_PENDING;
    cur->pend_ret_code = PEND_NONE;
    task_list_add_priority(&mutex->wait_list, cur);

    task_switch();

    leave_critical_section(state);

    return MUTEX_OK;
}

static int mutex_wait_timeout_cb(void *arg)
{
    task_t *task = (task_t*)arg;

    list_delete(&task->node);
    task_become_ready_head(task);
    KINFO("task %s mutex wait timeout\r\n",
            task->name);
    task->pend_ret_code = PEND_TIMEOUT;

    return 0;
}


int mutex_timedwait(mutex_t *mutex, unsigned long ticks)
{
    task_t *cur;
    irqstate_t state;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    if (mutex->cnt == 0) {
        mutex->cnt++;
        mutex->holder = cur;
        leave_critical_section(state);
        return MUTEX_OK;
    }

    if (mutex->holder == cur) {
        mutex->cnt++;
        leave_critical_section(state);
        return MUTEX_ENESTED;
    }

    if (mutex->holder->priority > cur->priority) {
        task_set_priority(mutex->holder, cur->priority);
    }

    cur->pending_list = &mutex->wait_list;
    cur->state = TASK_PENDING;
    cur->pend_ret_code = PEND_NONE;
    task_list_add_priority(&mutex->wait_list, cur);
    register_oneshot_timer(&cur->wait_timer, "mutex", ticks,
                            mutex_wait_timeout_cb, cur);

    task_switch();
    cur->pending_list = NULL;

    leave_critical_section(state);

    return MUTEX_OK;
}

int mutex_trywait(mutex_t *mutex)
{
    irqstate_t state;
    task_t *cur;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    if (mutex->cnt == 0) {
        mutex->cnt++;
        mutex->holder = cur;
        leave_critical_section(state);
        return MUTEX_OK;
    }

    if (mutex->holder == cur) {
        mutex->cnt++;
        leave_critical_section(state);
        return MUTEX_ENESTED;
    }

    leave_critical_section(state);

    return MUTEX_EAGAIN;
}

int mutex_post(mutex_t *mutex)
{
    irqstate_t state;
    task_t *cur;
    task_t *wakeup;
    int ret = MUTEX_OK;

    KASSERT(mutex != NULL);
    KASSERT(mutex->magic == MUTEX_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    if (!list_is_empty(&mutex->wait_list)) {
        wakeup = list_first_entry(&mutex->wait_list, task_t, node);
        list_delete(&wakeup->node);
        mutex->holder = wakeup;
        wakeup->pend_ret_code = PEND_WAKEUP;
        task_become_ready_head(wakeup);
        task_become_ready_tail(cur);
        task_switch();

        goto out;
    }

    if (mutex->cnt >= 1) {
        if (mutex->holder == cur) {
            mutex->cnt--;
            if (mutex->cnt == 0)
                mutex->holder = NULL;
        } else {
            ret = MUTEX_EHOLDER;
        }
        goto out;
    } else {
        ret = MUTEX_ECNT;
        goto out;
    }

out:
    leave_critical_section(state);
    return ret;
}

