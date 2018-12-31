#include <typedef.h>
#include <irq.h>
#include <sem.h>
#include <task.h>
#include <list.h>
#include <mm.h>
#include <timer.h>
#include <kdebug.h>
#include <string.h>

int sem_init(sem_t *sem, int val)
{
    int ret = SEM_OK;
    irqstate_t state;

    KASSERT(sem != NULL);

    memset(sem, 0, sizeof(sem_t));
    sem->magic = SEM_MAGIC;
    sem->cnt = val;
    list_head_init(&sem->wait_list);

    return ret;
}

int sem_wait(sem_t *sem)
{
    int ret = SEM_OK;
    irqstate_t state;

    KASSERT(sem != NULL);
    KASSERT(sem->magic == SEM_MAGIC);

    state = enter_critical_section();

    task_t *cur = get_cur_task();

    sem->cnt--;
    KDBG("task %s wait sem, val %d\r\n", cur->name, sem->cnt);

    if (sem->cnt < 0) {
        cur->state = TASK_PENDING;
        cur->pend_ret_code = PEND_NONE;
        cur->pending_list = &sem->wait_list;
        task_list_add_priority(&sem->wait_list, cur);
        task_switch();
        cur->pending_list = NULL;
    }

    leave_critical_section(state);

    return ret;
}

static int sem_wait_timeout_cb(void *arg)
{
    task_t *task = (task_t*)arg;

    list_delete(&task->node);
    task_become_ready_head(task);
    KINFO("task %s sem wait timeout\r\n",
            task->name);
    task->pend_ret_code = PEND_TIMEOUT;

    return 0;
}

/** If wait timeout, return SEM_TIMEOUT
 *  else return SEM_OK
 */
int sem_timedwait(sem_t *sem, int ms)
{
    int ret = SEM_OK;
    irqstate_t state;
    task_t *cur;

    KASSERT(sem != NULL);
    KASSERT(sem->magic == SEM_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    sem->cnt--;

    if (sem->cnt < 0) {
        cur->state = TASK_PENDING;
        cur->pend_ret_code = PEND_NONE;
        cur->pending_list = &sem->wait_list;
        task_list_add_priority(&sem->wait_list, cur);
        register_oneshot_timer(&cur->wait_timer, "sem", ms,
                            sem_wait_timeout_cb, cur);

        task_switch();
        cur->pending_list = NULL;

        if (cur->pend_ret_code == PEND_TIMEOUT) {
            sem->cnt++;
            ret = SEM_TIMEOUT;
        }
    }

    leave_critical_section(state);

    return ret;
}

int sem_trywait(sem_t *sem)
{
    int ret = SEM_OK;
    irqstate_t state;
    task_t *cur;

    KASSERT(sem != NULL);
    KASSERT(sem->magic == SEM_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    if (sem->cnt <= 0) {
        ret = SEM_AGAIN;
    } else {
        sem->cnt--;
    }

    leave_critical_section(state);

    return ret;
}

int sem_post(sem_t *sem)
{
    irqstate_t state;
    int ret = SEM_OK;
    task_t *cur;
    task_t *wakeup;

    KASSERT(sem != NULL);
    KASSERT(sem->magic == SEM_MAGIC);

    state = enter_critical_section();

    cur = get_cur_task();

    sem->cnt++;

    KDBG("task %s post sem, val %d\r\n", cur->name, sem->cnt);

    if (sem->cnt > 0) {
        ret = SEM_OK;
    } else {
        if (!list_is_empty(&sem->wait_list)) {
            wakeup = list_first_entry(&sem->wait_list, task_t, node);
            list_delete(&wakeup->node);
            task_become_ready_head(wakeup);
            task_become_ready_tail(cur);
            task_switch();
        }
    }
    leave_critical_section(state);

    return ret;
}

