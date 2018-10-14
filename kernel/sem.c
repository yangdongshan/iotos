#include <typedef.h>
#include <irq.h>
#include <sem.h>
#include <task.h>
#include <list.h>
#include <mm.h>
#include <timer.h>
#include <kdebug.h>
#include <string.h>


#ifdef CONFIG_PRE_ALLOC_HOLDER_CNT
#define PRE_ALLOC_HOLDER_CNT CONFIG_PRE_ALLOC_HOLDER_CNT
#else
#define PRE_ALLOC_HOLDER_CNT 20
#endif

#define FREE_HOLDER_CNT_HIGH_WATER (PRE_ALLOC_HOLDER_CNT + 20)

#define STATIC_ALLOC_HOLDER   0x0
#define DYNAMIC_ALLOC_HOLER   0x1

typedef struct sem_holder {
    struct list_node node;
    task_t *task;
    int cnt;
    int flag;
}sem_holder_t;

list_head_t free_holder_list;

static int free_holder_cnt;

static sem_holder_t pre_alloc_holder[PRE_ALLOC_HOLDER_CNT];

#ifdef CONFIG_DYNAMIC_ALLOC_HOLDER
static sem_holder_t* alloc_holder(void)
{
    int size;
    sem_holder_t *holder;

    size = sizeof(sem_holder_t);
    holder = (sem_holder_t*)malloc(size);
    if (holder == NULL)
        return NULL;

    memset(holder, 0, sizeof(sem_holder_t));
    holder->flag |= DYNAMIC_ALLOC_HOLER;

    return holder;
}
#endif

static sem_holder_t* get_holder(void)
{
    sem_holder_t *holder;

    if (list_is_empty(&free_holder_list)) {
#ifdef CONFIG_DYNAMIC_ALLOC_HOLDER
        holder = alloc_holder();
        return holder;
#else
        return NULL;
#endif
    }

    holder = list_first_entry(&free_holder_list, sem_holder_t, node);
    list_delete(&holder->node);
    free_holder_cnt--;

    return holder;
}

static void free_holder(sem_holder_t *holder)
{
    if ((holder->flag & DYNAMIC_ALLOC_HOLER) &&
        (free_holder_cnt > FREE_HOLDER_CNT_HIGH_WATER)) {
        free(holder);
    } else {
        list_add_tail(&free_holder_list, &holder->node);
        free_holder_cnt++;
    }
}

void sem_init_early(void)
{
    int i;

    list_head_init(&free_holder_list);

    memset(pre_alloc_holder, 0, sizeof(pre_alloc_holder));

    for (i = 0; i < PRE_ALLOC_HOLDER_CNT; i++) {
        list_add_tail(&free_holder_list, &pre_alloc_holder[i].node);
        pre_alloc_holder[i].flag |= STATIC_ALLOC_HOLDER;
    }

    free_holder_cnt = PRE_ALLOC_HOLDER_CNT;
}

int sem_init(sem_t *sem, int val)
{
    int ret = SEM_OK;
    irqstate_t state;

    KASSERT(sem != NULL);

    memset(sem, 0, sizeof(sem_t));

    state = enter_critical_section();

    list_head_init(&sem->wait_list);
    list_head_init(&sem->holder_list);
    sem->cnt = val;

    leave_critical_section(state);

    return ret;
}

static int sem_add_holder(sem_t *sem, task_t *task)
{
    int ret = -1;
    sem_holder_t *holder;
    struct list_node *iter_node;
    bool found = false;

    if (!list_is_empty(&sem->holder_list)) {
        iter_node = sem->holder_list.next;
        while (iter_node != &sem->holder_list) {
            holder = list_entry(iter_node, sem_holder_t, node);
            if (holder->task == task) {
                holder->cnt++;
                sem->holder_cnt++;
                ret = 0;
                found = true;
                break;
            }
            iter_node = iter_node->next;
        }
    }

    if (found == false) {
        holder = get_holder();
        if (holder != NULL) {
            holder->task = task;
            list_add_tail(&sem->holder_list, &holder->node);
            holder->cnt++;
            sem->holder_cnt++;
            ret = 0;
        }
    }

    return ret;
}

static int sem_remove_holder(sem_t *sem, task_t *task)
{
    int ret = -1;
    struct list_node *cur_node, *next_node;
    sem_holder_t *holder;

    KASSERT(sem != NULL);
    KASSERT(task != NULL);

    next_node = sem->holder_list.next;
    while (next_node != &sem->holder_list) {
        cur_node = next_node;
        next_node = cur_node->next;
        holder = list_entry(cur_node, sem_holder_t, node);
        if (holder->task == task) {
            sem->holder_cnt--;
            holder->cnt--;
            if (holder->cnt == 0) {
                list_delete(&holder->node);
                free_holder(holder);
            }
            ret = 0;
            break;
        }
    }

    return ret;
}

int sem_wait(sem_t *sem)
{
    int ret = SEM_OK;
    irqstate_t state;

    KASSERT(sem != NULL);

    state = enter_critical_section();

    task_t *cur = get_cur_task();

    sem->cnt--;
    KDBG("task %s wait sem, val %d\r\n", cur->name, sem->cnt);

    if (sem->cnt >= 0) {
        if (sem_add_holder(sem, cur)) {
            ret = SEM_HOLDER_ERR;
        }
    } else {
        // FIXME: priority inherit
        list_add_tail(&sem->wait_list, &cur->node);
        cur->state = TASK_PENDING;
        task_switch();
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

    KASSERT(sem != NULL);

    state = enter_critical_section();

    task_t *cur = get_cur_task();

    sem->cnt--;

    if (sem->cnt >= 0) {
        if (sem_add_holder(sem, cur)) {
            ret = SEM_HOLDER_ERR;
        }
    } else {
        // FIXME: priority inherit
        list_add_tail(&sem->wait_list, &cur->node);
        cur->state = TASK_PENDING;
        cur->pend_ret_code = PEND_NONE;
        sem->timer = register_oneshot_timer(&cur->wait_timer, "sem", ms,
                            sem_wait_timeout_cb, cur);
        task_switch();
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

    state = enter_critical_section();

    cur = get_cur_task();

    if (sem->cnt <= 0) {
        ret = SEM_AGAIN;
    } else {
        sem->cnt--;
        if (sem_add_holder(sem, cur)) {
            ret = SEM_HOLDER_ERR;
        }
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

    state = enter_critical_section();

    cur = get_cur_task();

    sem->cnt++;

    KDBG("task %s post sem, val %d\r\n", cur->name, sem->cnt);

    /** remove the task from sem holder,
    * if the sem is taken by cur task.
    */
    sem_remove_holder(sem, cur);

    if (sem->cnt > 0) {
        ret = SEM_OK;
    } else {
        if (!list_is_empty(&sem->wait_list)) {
            wakeup = list_first_entry(&sem->wait_list, task_t, node);
            sem_add_holder(sem, wakeup);
            list_delete(&wakeup->node);
            task_become_ready_head(wakeup);
            if (sem->timer != NULL) {
                cancel_timer(sem->timer);
                sem->timer = NULL;
            }


            task_become_ready_tail(cur);
            task_switch();
        }
    }
    leave_critical_section(state);

    return ret;
}

