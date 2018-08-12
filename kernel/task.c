#include <string.h>
#include <err.h>
#include <irq.h>
#include <kernel.h>
#include <kdebug.h>


#define to_task_ptr(node) container_of(node, struct list_node, node)

#define BITS_U32_CNT(bits) ((bits + 31)/32)

static task_t *task_id_table[MAX_TASK_CNT];
static task_t *cur_task = NULL;

static int idle_task_id = -1;

static struct list_node global_task_list;
static struct list_node task_ready_list[LOWEST_TASK_PRIORITY + 1];

static struct list_node task_suspend_list;
static struct list_node task_pending_list;

char *g_cur_task_stack_ptr;
char *g_new_task_stack_ptr;

// FIXME assume max task priority is not more than 32
#define RUNQUEUE_WORD BITS_U32_CNT(LOWEST_TASK_PRIORITY + 1)
static uint32_t runqueue_bitmap[RUNQUEUE_WORD];

static int get_runqueue_first_set_bit(void)
{
    int i;
    int bit = 0;

    for (i = 0; i < RUNQUEUE_WORD; i++) {
        if (runqueue_bitmap[i] != 0)
            break;
    }

    if (i == RUNQUEUE_WORD)
        return -1;

    int word = runqueue_bitmap[i];

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

    KDBG("set runqueue word %d bit %d for priority %d\r\n", word, bit, priority);
    runqueue_bitmap[word] |= (1 << bit);
}

static void clear_runqueue_bit(int priority)
{
    KASSERT((priority >= 0) && (priority <= LOWEST_TASK_PRIORITY));

    uint8_t word = priority / 32;
    uint8_t bit = priority % 32;

    runqueue_bitmap[word] &= ~(1 << bit);
    KDBG("clear runqueue word %d bit %d for priority %d\r\n", word, bit, priority);

}
static inline int get_idle_task_id(void)
{
    return idle_task_id;
}

void set_idle_task_id(int id)
{
    idle_task_id = id;
}

static inline task_t* idle_task(void)
{
    return task_id_table[idle_task_id];
}

static inline task_t* get_task_by_id(int task_id)
{
    if (task_id < 0 || task_id > MAX_TASK_ID)
        return NULL;

    return task_id_table[task_id];
}

static inline bool task_id_valid(int th_id)
{
    if (th_id >= 0 && th_id <= MAX_TASK_ID)
        return true;
    else
        return false;
}

static inline bool task_is_valid(int task_id)
{
    if (((task_id >= 0) && (task_id <= MAX_TASK_ID))
        || task_id == IDLE_TASK_ID) {
        return true;
    } else {
        return false;
    }
}

static void task_set_name(task_t *task, const char *name)
{
    irqstate_t state;

    size_t name_len = strlen(name);
    if (name_len >= MAX_TASK_NAME_LEN) {
        name_len = MAX_TASK_NAME_LEN - 1;
    }

    memset(task->name, 0, MAX_TASK_NAME_LEN);

    state = enter_critical_section();
    strncpy(task->name, name, name_len);
    leave_critical_section(state);

}

static void task_set_priority(task_t *task, int priority)
{
    irqstate_t state;

    state = enter_critical_section();

    task->priority = priority;

    leave_critical_section(state);
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
    return cur_task;
}

static inline void set_cur_task(task_t *task)
{
    irqstate_t state;

    state = enter_critical_section();
    KDBG("set cur_task %s\r\n", task->name);
    cur_task = task;
    leave_critical_section(state);
}

static int task_allocate_id(task_t *task)
{
    irqstate_t state;
    int i;
    int id = -1;

    state = enter_critical_section();

    for (i = 0; i < MAX_TASK_CNT; i++) {
        if (task_id_table[i] == NULL) {
            id = i;
            task_id_table[id] = task;
            task->task_id = id;
            break;
        }
    }

    leave_critical_section(state);
    return id;
}

void task_init(void)
{
    unsigned int i;

    memset(task_id_table, 0, sizeof(task_id_table));
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
    int ret = NO_ERR;
    task_t *task = (task_t*)arg;

    if (task->main_entry) {
        ret = task->main_entry(task->main_arg);
    }

    return ret;
}

static void task_setup_initial_stack(task_t *task)
{
    struct context_frame *cf;

    addr_t stack_top = (addr_t)task->sp_alloc_addr + task->stack_size;

    // align at 8 bytes
    stack_top = stack_top & (~0x07ul);
    task->stack_size = stack_top - (addr_t)task->sp_alloc_addr;

    cf = (struct context_frame*)stack_top;
    cf--;

    arch_init_context_frame(cf, (addr_t*)task->start_entry, task->start_arg);

    task->sp = (addr_t)cf;
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
    irqstate_t state;

    KDBG("task %s state %d insert into ready list head\r\n",
            task->name, task->state);

    KASSERT(task->state != TASK_READY);
    KASSERT(!list_in_list(&task->node));

    task->state = TASK_READY;
    task_addto_ready_list_head(task);
}

void task_become_ready_tail(task_t *task)
{
    irqstate_t state;

    KDBG("task %s state %d insert into ready list tail\r\n",
            task->name, task->state);

    KASSERT(task->state != TASK_READY);
    KASSERT(!list_in_list(&task->node));

    task->state = TASK_READY;
    task_addto_ready_list_tail(task);
}

int task_suspend(int task_id)
{
    irqstate_t state;
    bool resched = false;

    if (task_id_valid(task_id))
        return -ERR_INVALID_TASK_ID;

    if (task_id == idle_task_id)
        return -ERR_SUSPEDN_IDLE_TASK;

    task_t *task = get_task_by_id(task_id);

    state = enter_critical_section();

    if (task->state == TASK_RUNNING) {
        resched = true;
    }

    task_addto_suspend_list(task);

    if (resched)
        task_sched();

    leave_critical_section(state);


    return NO_ERR;
}


void task_yield(void)
{
    irqstate_t state;
    task_t *task;

    state = enter_critical_section();

    task= get_cur_task();
    task->state = TASK_READY;
    task_addto_ready_list_tail(task);
    task_sched();

    leave_critical_section(state);

}

int task_resume(task_t *task)
{
    irqstate_t state;
    bool resched = false;

    if (task == NULL)
        return NO_ERR;

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

    return NO_ERR;
}

int task_exit(int task_id)
{
    return NO_ERR;
}

int task_join(int task_id)
{
    return NO_ERR;
}


task_t* get_new_task()
{
    task_t *task;

    int priority = get_runqueue_first_set_bit();

    // no task other than idle is ready
    if (priority == -1) {
        return NULL;
    }

    KASSERT(!list_is_empty(&task_ready_list[priority]));

    task = list_first_entry(&task_ready_list[priority], struct task, node);
    list_delete(&task->node);
    if (list_is_empty(&task_ready_list[priority])) {
        clear_runqueue_bit(priority);
    }

out:
    return task;
}

bool task_can_be_preempted(void)
{
    task_t *cur = get_cur_task();
    int highest = get_runqueue_first_set_bit();

    if (cur && (highest < cur->priority)) {
        return true;
    } else {
        return false;
    }
}

// should be invoked with interrupt disabled
void task_sched(void)
{
    if (in_nested_interrupt())
        return;

    task_t *old = get_cur_task();
    task_t *new = get_new_task();

    if (new == NULL) {
        KINFO("no ready task in ready queue\r\n");
        return;
    }

    new->state = TASK_RUNNING;

    if (new == old)
        return;

    KDBG("%s, old task %s, new task %s\r\n",
            __func__, old->name, new->name);

    set_cur_task(new);
    g_new_task_stack_ptr = &new->sp;
    g_cur_task_stack_ptr = &old->sp;
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
    int ret = NO_ERR;

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

    if (-1 == task_allocate_id(task)) {
        return -ERR_NO_TASK_ID_AVAILABLE;
    }

    task_setup_initial_stack(task);

    state = enter_critical_section();

    if (TASK_IS_AUTO_RUN(task->flags)) {
        task_become_ready_tail(task);
    }
    task_addto_suspend_list(task);

    leave_critical_section(state);

    return NO_ERR;
}


void task_sched_start(void)
{
    task_t *task, *temp;
    irqstate_t state;

    state = enter_critical_section();

    list_foreach_entry_safe(&task_suspend_list, task, temp, struct task, node) {
            list_delete(&task->node);
            task->state = TASK_READY;
            task_addto_ready_list_tail(task);
    }
    task = get_new_task();
    task->state = TASK_RUNNING;
    set_cur_task(task);
    leave_critical_section(state);

    g_new_task_stack_ptr = &task->sp;
    arch_start_first_task();

    KERR("shouldn't return here\r\n");
    KASSERT(0);
}

