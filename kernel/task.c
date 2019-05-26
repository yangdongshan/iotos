#include <string.h>
#include <err.h>
#include <irq.h>
#include <task.h>
#include <sched.h>
#include <kdebug.h>

#define to_task_ptr(node) container_of(node, struct list_node, node)

#define BITS_TO_WORD(bits) ((bits + 31)/32)

list_head_t g_task_list;

list_head_t g_ready_list[LOWEST_TASK_PRIORITY + 1];

list_head_t g_suspend_list;

task_t *g_cur_task = NULL;
task_t *g_new_task = NULL;

// FIXME assume max task priority is not more than 32
#define RUNQUEUE_WORD BITS_TO_WORD(LOWEST_TASK_PRIORITY + 1)

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

static void set_runqueue_bit(int priority)
{
    KASSERT((priority >= 0) && (priority <= LOWEST_TASK_PRIORITY));

    uint8_t word = priority / 32;
    uint8_t bit = priority % 32;

    runqueue_bitmap[word] |= (1 << bit);
}

static void clear_runqueue_bit(int priority)
{
    KASSERT((priority >= 0) && (priority <= LOWEST_TASK_PRIORITY));

    uint8_t word = priority / 32;
    uint8_t bit = priority % 32;

    runqueue_bitmap[word] &= ~(1 << bit);
}

void task_list_add_priority(struct list_node *head, task_t *task)
{
    task_t *iter;

    if (list_is_empty(head)){
        list_add_head(head, &task->node);
        return;
    }

    list_foreach_entry(head, iter, task_t, node) {
        if (iter->prio < task->prio) {
            list_add_before(&iter->node, &task->node);
            return;
        }
    }

    list_add_tail(head, &task->node);
}

void task_list_add_head(struct list_node *head, task_t *task)
{
    list_add_head(head, &task->node);
}

void task_list_add_tail(struct list_node *head, task_t *task)
{
    list_add_tail(head, &task->node);
}

static void task_addto_ready_list_head(task_t *task)
{

    KASSERT(task->state == TS_READY);
    KASSERT(!list_in_list(&task->node));

    list_add_head(&g_ready_list[task->prio], &task->node);
    set_runqueue_bit(task->prio);
}

void task_addto_ready_list_tail(task_t *task)
{

    KASSERT(task->state == TS_READY);
    KASSERT(!list_in_list(&task->node));

    list_add_tail(&g_ready_list[task->prio], &task->node);
    set_runqueue_bit(task->prio);
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

void task_set_priority(task_t *task, int prio)
{
    switch (task->state) {
        case TS_READY:
            list_delete(&task->node);
            clear_runqueue_bit(task->origin_prio);
            task->prio = prio;
            task_addto_ready_list_tail(task);
            break;

        default:
            task->prio = prio;
            break;
    }

}

void task_restore_priority(task_t *task)
{
    switch (task->state) {
        case TS_READY:
            list_delete(&task->node);
            clear_runqueue_bit(task->prio);
            task->prio = task->origin_prio;
            task_addto_ready_list_tail(task);
            break;

        default:
            task->prio = task->origin_prio;
            break;
    }
}

void task_init_early(void)
{
    unsigned int i;

    memset(runqueue_bitmap, 0, sizeof(runqueue_bitmap));

    for (i = 0; i < LOWEST_TASK_PRIORITY + 1; i++) {
        list_head_init(&g_ready_list[i]);
    }

	list_head_init(&g_suspend_list);
	list_head_init(&g_task_list);
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

    task->state = TS_SUSPENDED;
    list_add_tail(&g_suspend_list, &task->node);

}


static void task_become_suspended(task_t *task)
{
    KDBG("task %s state %d insert into suspended list\r\n",
            task->name, task->state);

    KASSERT(task->state != TS_SUSPENDED);
    KASSERT(!list_in_list(&task->node));

    task->state = TS_SUSPENDED;
    list_add_tail(&g_suspend_list, &task->node);
    set_runqueue_bit(task->prio);
}


/* this function should be called with interrupt disabled
 */
void task_become_ready(task_t *task)
{
    KDBG("task %s state %d insert into ready list tail\r\n",
            task->name, task->state);

    KASSERT(task->state != TS_READY);
    KASSERT(!list_in_list(&task->node));

    task->state = TS_READY;
    list_add_tail(&g_ready_list[task->prio], &task->node);
    set_runqueue_bit(task->prio);
}

static void task_ready_list_remove(task_t *task)
{
    KASSERT(task->state == TS_READY);
    KASSERT(!list_in_list(&task->node));

    list_delete(&task->node);
    if (list_is_empty(&g_ready_list[task->prio])) {
        clear_runqueue_bit(task->prio);
    }
}

int task_wakeup(task_t *task)
{
    irqstate_t state;

    KASSERT(task != NULL);

    state = enter_critical_section();

    leave_critical_section(state);

    return ERR_OK;
}





int _task_suspend(task_t *task)
{
    irqstate_t state;
    int resched = 0;

    switch (task->state) {
		case TS_INIT:
			break;
        case TS_READY:
            list_delete(&task->node);
			break;
        case TS_RUNNING:
			list_delete(&task->node);
			resched = 1;
			break;
        case TS_PEND_SEM:
		case TS_PEND_MUTEX:
			list_delete(&task->node);
			tick_cancel(task);
			break;
        case TS_SUSPENDED:
        default:
            goto out;
    }


	list_add_tail(&g_suspend_list, &task->node);
	task->state = TS_SUSPENDED;

out:

    return resched;
}

int task_suspend(task_t *task)
{
	irqstate_t state;
	int ret;
	
	state = enter_critical_section();
	ret = _task_suspend(task);
	leave_critical_section(state);

	return ret;
}

void task_yield(void)
{
    irqstate_t state;
    task_t *task;

    state = enter_critical_section();

    task= get_cur_task();
    task->state = TS_READY;
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

    if (task->state == TS_SUSPENDED) {
        list_delete(&task->node);
        task->state = TS_READY;
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


task_t* get_prefer_task(void)
{
    task_t *task;

    int highest_prio = get_prefer_task_priority();

	KASSERT(highest_prio >= 0 && highest_prio <= LOWEST_TASK_PRIORITY);

    KASSERT(!list_is_empty(&g_ready_list[highest_prio]));

    task = list_first_entry(&g_ready_list[highest_prio], task_t, node);

    return task;
}

int task_need_resched(void)
{
	task_t *cur = get_cur_task;
	int highest_prio = get_prefer_task_priority();

	return (highest_prio < cur->prio)? 1: 0;
}

int task_sleep(tick_t ticks)
{
	irqstate_t state;
	task_t *cur;
	tick_t now;
	int ret;
	
	state = enter_critical_section();

	cur = get_cur_task();
	now = get_sys_tick();

	cur->pend_time = now;
	cur->pend_timeout = now + ticks;
	cur->pend_ret_code = PEND_OK;
	cur->state = TS_PEND_SLEEP;
    tick_list_insert(cur);
	ret = task_switch();
	leave_critical_section(state);

	if (ret < 0) {
		return ret;
	}
	
	state = enter_critical_section();
	ret = cur->pend_ret_code;
	leave_critical_section(state);

	if (ret == PEND_TIMEOUT)
		return ERR_OK;

	return ERR_OK;
}

/** 1. should be invoked with interrupt disabled
 *  2. cur task should be already handled, it shouldn't be in ready list
 */
int task_switch(void)
{
    if (is_interrupt_nested())
        return -ERR_INT_NESTED;

	if (IS_SCHED_LOCKED())
		return -ERR_SCHED_LOCKED;

    task_t *cur = get_cur_task();
    task_t *new = get_prefer_task();

    new->state = TS_RUNNING;

    if (new == cur)
        return;

    new->time_remain = new->time_slice;
    g_new_task = new;
    arch_context_switch();

	return ERR_OK;
}


void task_int_switch(void)
{

    task_t *cur = get_cur_task();
    task_t *new = get_prefer_task();

    if (new == NULL) {
        return;
    }

    new->state = TS_RUNNING;

    if (new == cur)
        return;

    new->time_remain = new->time_slice;
    g_new_task = new;
    arch_context_switch();
}


int task_create(task_t *task,
                const char* name,
                int prio,
                task_entry_t entry,
                void *arg,
                void *stack,
                size_t stack_size,
                unsigned int time_slice,
                unsigned int flags)
{
    irqstate_t state;

    if (task == NULL) {
        return -ERR_INV_ARG;
    }

    if (entry == NULL) {
        return -ERR_INV_ARG;
    }

	if (prio > LOWEST_TASK_PRIORITY || prio < 0) {
		return -ERR_INV_ARG;
	}

    memset(task, 0, sizeof(*task));
    memset(stack, 0, stack_size);

    task->name = name;
    task->prio = prio;
    task->origin_prio = prio;
    task->start_entry = start_entry;
    task->start_arg = (void*)task;
    task->main_entry = entry;
    task->main_arg = arg;
    task->sp_alloc_addr = stack;
    task->stack_size = stack_size;
    if (time_slice <= 0)
        time_slice = TASK_DEFAULT_TIME_SLICE;
    task->time_slice = time_slice;
    task->time_remain = time_slice;
    task->flags = flags;
	task->state = TS_INIT;

    task_setup_initial_stack(task);

    state = enter_critical_section();

    if ((task->flags & TF_AUTO_RUN) != 0) {
        task_become_ready(task);
    } else {
       	list_add_tail(&g_suspend_list, &task->node);
		task->state = TS_SUSPENDED;
    }

    leave_critical_section(state);

    return ERR_OK;
}

