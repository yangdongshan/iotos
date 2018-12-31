#include <string.h>
#include <err.h>
#include <irq.h>
#include <task.h>
#include <sched.h>
#include <kdebug.h>


#define to_task_ptr(node) container_of(node, struct list_node, node)

#define BITS_U32_CNT(bits) ((bits + 31)/32)

static struct list_node global_task_list;
static struct list_node task_ready_list[LOWEST_TASK_PRIORITY + 1];

static struct list_node task_suspend_list;
static struct list_node task_pending_list;

task_t *g_cur_task = NULL;
task_t *g_new_task = NULL;

// FIXME assume max task priority is not more than 32
#define RUNQUEUE_WORD BITS_U32_CNT(LOWEST_TASK_PRIORITY + 1)
static uint32_t runqueue_bitmap[RUNQUEUE_WORD];

static int get_prefer_task_priority(void)
{
    int i;
    int bit;

    for (i = 0; i < RUNQUEUE_WORD; i++) {
        if (runqueue_bitmap[i] != 0)
            break;
    }

    if (i == RUNQUEUE_WORD)
        return -1;

    int word = runqueue_bitmap[i];

    bit = 0;
    while (1) {
        if (word & (1 << bit))
            break;
        else
            bit++;
    }

    return i * 32 + bit;
}

void set_runqueue_bit(int priority)
{
    KASSERT((priority >= 0) && (priority <= LOWEST_TASK_PRIORITY));

    uint8_t word = priority / 32;
    uint8_t bit = priority % 32;

    runqueue_bitmap[word] |= (1 << bit);
}

void task_list_add_priority(struct list_node *head, task_t *task)
{
    task_t *iter;

    if (list_is_empty(head)){
        list_add_head(head, &task->node);
        return;
    }

    list_foreach_entry(head, iter, task_t, node) {
        if (iter->priority < task->priority) {
            list_add_before(&iter->node, &task->node);
            return;
        }
    }

    list_add_tail(head, &task->node);
}


static void clear_runqueue_bit(int priority)
{
    KASSERT((priority >= 0) && (priority <= LOWEST_TASK_PRIORITY));

    uint8_t word = priority / 32;
    uint8_t bit = priority % 32;

    runqueue_bitmap[word] &= ~(1 << bit);
}



static void task_addto_ready_list_head(task_t *task)
{

    KASSERT(task->state == TASK_READY);
    KASSERT(!list_in_list(&task->node));

    list_add_head(&task_ready_list[task->priority], &task->node);
    set_runqueue_bit(task->priority);
}

static void task_addto_ready_list_tail(task_t *task)
{

    KASSERT(task->state == TASK_READY);
    KASSERT(!list_in_list(&task->node));

    list_add_tail(&task_ready_list[task->priority], &task->node);
    set_runqueue_bit(task->priority);
}

task_t *get_cur_task(void)
{
    return g_cur_task;
}
/*
static inline void set_cur_task(task_t *task)
{
    irqstate_t state;

    state = enter_critical_section();
    g_cur_task = task;
    leave_critical_section(state);
}
*/

void task_set_priority(task_t *task, int priority)
{
    switch (task->state) {
        case TASK_READY:
            list_delete(&task->node);
            clear_runqueue_bit(task->origin_priority);
            task->priority = priority;
            task_addto_ready_list_tail(task);
            break;
        case TASK_PENDING:
            task->priority = priority;
            if (task->pending_list) {
                list_delete(&task->node);
                task_list_add_priority(task->pending_list, task);
            }
            break;

        default:
            task->priority = priority;
            break;
    }

}

void task_restore_priority(task_t *task)
{
    switch (task->state) {
        case TASK_READY:
            list_delete(&task->node);
            clear_runqueue_bit(task->priority);
            task->priority = task->origin_priority;
            task_addto_ready_list_tail(task);
            break;
        case TASK_PENDING:
            task->priority = task->origin_priority;
            if (task->pending_list) {
                list_delete(&task->node);
                task_list_add_priority(task->pending_list, task);
            }
            break;

        default:
            task->priority = task->origin_priority;
            break;
    }
}
void task_init_early(void)
{
    unsigned int i;

    memset(runqueue_bitmap, 0, sizeof(runqueue_bitmap));

    for (i = 0; i <= LOWEST_TASK_PRIORITY; i++) {
        list_head_init(&task_ready_list[i]);
    }

    list_head_init(&global_task_list);
    list_head_init(&task_suspend_list);
    list_head_init(&task_pending_list);
}

static int start_entry(void *arg)
{
    int ret = ERR_OK;
    task_t *task = (task_t*)arg;

    if (task->main_entry) {
        ret = task->main_entry(task->main_arg);
    }

    return ret;
}

static void task_grave(void)
{
    task_t *cur = get_cur_task();

    KASSERT(0);
}

static void task_setup_initial_stack(task_t *task)
{
    struct context_frame *cf;

    addr_t stack_top = (addr_t)task->sp_alloc_addr + task->stack_size;

    // align at 16 bytes
    stack_top = stack_top & (~0x0ful);
    task->stack_size = stack_top - (addr_t)task->sp_alloc_addr;

    cf = (struct context_frame*)stack_top;
    cf--;

    arch_init_context_frame(cf, (addr_t*)task->start_entry,
                            task->start_arg, (addr_t*)task_grave);

    task->stack = (addr_t)cf;
}


static inline void task_addto_suspend_list(task_t *task)
{
    KASSERT(!list_in_list(&task->node));

    list_add_tail(&task_suspend_list, &task->node);
    task->state = TASK_SUSPENDED;
}

static inline void merge_pending_task_to_ready_list(void)
{
    task_t *task;

    while (!list_is_empty(&task_suspend_list)) {
            task = list_first_entry(&task_suspend_list, task_t, node);
            list_delete(&task->node);
            KDBG("merge task %s to ready list\r\n", task->name);
            task_become_ready_tail(task);
    }
}

/* this function should be called with interrupt disabled
 */
void task_become_ready_head(task_t *task)
{
    KDBG("task %s state %d insert into ready list head\r\n",
            task->name, task->state);

    KASSERT(task->state != TASK_READY);
    KASSERT(!list_in_list(&task->node));

    task->state = TASK_READY;
    task_addto_ready_list_head(task);
}

void task_become_ready_tail(task_t *task)
{
    KDBG("task %s state %d insert into ready list tail\r\n",
            task->name, task->state);

    KASSERT(task->state != TASK_READY);
    KASSERT(!list_in_list(&task->node));

    task->state = TASK_READY;
    task_addto_ready_list_tail(task);
}

int task_wakeup(task_t *task)
{
    irqstate_t state;

    KASSERT(task != NULL);

    state = enter_critical_section();

    KASSERT(task->state != TASK_SLEEPING);

    kdebug_print("%s: task state %d\r\n", __func__, task->state);
    switch (task->state) {
        case TASK_SLEEPING:
            cancel_timer(&task->wait_timer);
            break;
        case TASK_PENDING:
            break;
        default:
            goto out;
    }

    task->pend_ret_code = PEND_WAKEUP;
    list_delete(&task->node);
    task->state = TASK_READY;
    task_addto_ready_list_head(task);
    task_switch();

out:
    leave_critical_section(state);

    return ERR_OK;
}

int task_suspend(task_t *task)
{
    irqstate_t state;
    bool resched = false;

    state = enter_critical_section();

    if (task->state == TASK_RUNNING) {
        resched = true;
    }

    task_addto_suspend_list(task);

    if (resched)
        task_switch();

    leave_critical_section(state);


    return ERR_OK;
}


void task_yield(void)
{
    irqstate_t state;
    task_t *task;

    state = enter_critical_section();

    task= get_cur_task();
    task->state = TASK_READY;
    task_addto_ready_list_tail(task);
    task_switch();

    leave_critical_section(state);
}

int task_resume(task_t *task)
{
    irqstate_t state;
    bool resched = false;

    if (task == NULL)
        return ERR_OK;

    state = enter_critical_section();

    if (task->state == TASK_SUSPENDED) {
        list_delete(&task->node);
        task->state = TASK_READY;
        task_addto_ready_list_head(task);
        resched = true;
    }

    leave_critical_section(state);

    if (resched == true) {
        task_yield();
    }

    return ERR_OK;
}

int task_exit(int task_id)
{
    return ERR_OK;
}

int task_join(int task_id)
{
    return ERR_OK;
}


task_t* get_new_task()
{
    task_t *task;

    int priority = get_prefer_task_priority();

    // no task other than idle is ready
    if (priority == -1) {
        return NULL;
    }

    KASSERT(!list_is_empty(&task_ready_list[priority]));

    task = list_first_entry(&task_ready_list[priority], task_t, node);
    list_delete(&task->node);
    if (list_is_empty(&task_ready_list[priority])) {
        clear_runqueue_bit(priority);
    }

    return task;
}

bool task_can_be_preempted(void)
{
    task_t *cur = get_cur_task();
    int highest = get_prefer_task_priority();

    if (cur && (highest < cur->priority)) {
        return true;
    } else {
        return false;
    }
}

// should be invoked with interrupt disabled
void task_switch(void)
{
    if (in_nested_interrupt())
        return;

    task_t *old = get_cur_task();
    task_t *new = get_new_task();

    if (new == NULL) {
        return;
    }

    new->state = TASK_RUNNING;

    if (new == old)
        return;

    new->time_remain = new->time_slice;
    g_new_task = new;
    arch_context_switch();
}

int task_create(task_t *task,
                const char* name,
                unsigned int priority,
                task_main_t main_entry,
                void *arg,
                void *stack,
                size_t stack_size,
                tick_t time_slice,
                unsigned int flags)
{
    irqstate_t state;

    if (task == NULL) {
        return -ERR_INVALID_TASK_ENTRY;
    }

    if (main_entry == NULL) {
        return -ERR_INVALID_TASK_ENTRY;
    }

    memset(task, 0, sizeof(*task));
    memset(stack, 0, stack_size);

    task->name = name;
    task->priority = priority;
    task->origin_priority = priority;
    task->start_entry = start_entry;
    task->start_arg = task;
    task->main_entry = main_entry;
    task->main_arg = arg;
    task->sp_alloc_addr = stack;
    task->stack_size = stack_size;
    if (time_slice <= 0)
        time_slice = TASK_DEFAULT_TIME_SLICE;
    task->time_slice = time_slice;
    task->time_remain = time_slice;
    task->flags = flags;

    task_setup_initial_stack(task);

    state = enter_critical_section();

    if (TASK_IS_AUTO_RUN(task->flags)) {
        task_become_ready_tail(task);
    } else {
        task_addto_suspend_list(task);
    }

    leave_critical_section(state);

    return ERR_OK;
}

