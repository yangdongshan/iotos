#include <string.h>
#include <errno.h>
#include <kernel.h>
#include <kdebug.h>

#define to_thread_ptr(node) container_of(node, struct list_node, node)

#define BITS_U8_CNT(bits) ((bits + 7)/8)

static thread_t *thread_id_table[MAX_THREAD_CNT];
static thread_t *cur_thread = NULL;

static thread_t main_thread;
static thread_t *idle_thread = NULL;

static struct list_node global_thread_list;
static struct list_node thread_ready_list[LOWEST_THREAD_PRIORITY + 1];

static struct list_node thread_suspend_list;

// FIXME assume max thread priority is not more than 32
#define RUNQUEUE_SIZE BITS_U8_CNT(LOWEST_THREAD_PRIORITY + 1)
static uint8_t runqueue_bitmap[RUNQUEUE_SIZE];

static int get_runqueue_first_set_bit(void)
{
    int byte, bit;
    uint8_t ch;

    for (byte = 0; byte < RUNQUEUE_SIZE; byte++) {
        if (runqueue_bitmap[byte] != 0)
            break;
    }

    if (byte == RUNQUEUE_SIZE)
        return -1;

    ch = runqueue_bitmap[byte];
        if (ch & 0x01) {
            bit = 0;
        } else if (ch & 0x02) {
            bit = 1;
        } else if (ch & 0x04) {
            bit = 2;
        } else if (ch & 0x08){
            bit = 3;
        } else if (ch & 0x10) {
            bit = 4;
        } else if (ch & 0x20) {
            bit = 5;
        } else if (ch & 0x40) {
            bit = 6;
        } else if (ch & 0x80) {
            bit = 7;
        }

    return byte * 8 + bit;
}

void set_runqueue_bit(int priority)
{

    KASSERT(priority >= 0 && priority <= LOWEST_THREAD_PRIORITY);

    uint8_t byte = priority / 8;
    uint8_t bit = byte % 8;

    runqueue_bitmap[byte] |= 1 << bit;
}


static inline thread_t* get_thread_by_id(int thread_id)
{
    if (thread_id < 0 || thread_id > MAX_THREAD_ID)
        return NULL;

    return thread_id_table[thread_id];
}

static inline bool thread_id_valid(int th_id)
{
    if (th_id >= 0 && th_id <= MAX_THREAD_ID)
        return true;
    else
        return false;
}

static inline bool thread_is_valid(int thread_id)
{
    if (thread_id >= 0 && thread_id <= MAX_THREAD_ID
        || thread_id == IDLE_THREAD_ID) {
        return true;
    } else {
        return false;
    }
}

static void thread_set_name(thread_t *thread, const char *name)
{
    irqstate_t state;

    size_t name_len = strlen(name);
    if (name_len >= MAX_THREAD_NAME_LEN) {
        name_len = MAX_THREAD_NAME_LEN - 1;
        memset(thread->name, 0, MAX_THREAD_NAME_LEN);

        state = enter_critical_section();
        strncpy(thread->name, name, name_len);
        leave_critical_section(state);

    }
}

static void thread_set_priority(thread_t *thread, int priority)
{
    irqstate_t state;

    state = enter_critical_section();

    thread->priority = priority;

    leave_critical_section(state);
}

static void thread_insert_into_ready_list_head(thread_t *thread)
{

    KASSERT(thread->state == THREAD_READY);
    KASSERT(!list_in_list(&thread->node));

    list_add_head(&thread_ready_list[thread->priority], &thread->node);
    set_runqueue_bit(1 << thread->priority);
}

static void thread_insert_into_ready_list_tail(thread_t *thread)
{

    KASSERT(thread->state == THREAD_READY);
    KASSERT(!list_in_list(&thread->node));

    list_add_tail(&thread_ready_list[thread->priority], &thread->node);
    set_runqueue_bit(1 << thread->priority);
}

static inline thread_t *get_cur_thread(void)
{
    return cur_thread;
}

static inline void set_cur_thread(thread_t *thread)
{
    irqstate_t state;

    state = enter_critical_section();
    cur_thread = thread;
    leave_critical_section(state);
}

static int thread_allocate_id(thread_t *thread)
{
    irqstate_t state;
    int i;
    int id = -1;

    state = enter_critical_section();

    for (i = 0; i < MAX_THREAD_CNT; i++) {
        if (thread_id_table[i] == NULL) {
            id = i;
            thread_id_table[id] = thread;
            thread->thread_id = id;
            break;
        }
    }

    leave_critical_section(state);
    return id;
}

void thread_early_init(void)
{
    unsigned int i;
    thread_t *thread;

    memset(thread_id_table, 0, sizeof(thread_id_table));
    memset(runqueue_bitmap, 0, sizeof(runqueue_bitmap));

    for (i = 0; i <= LOWEST_THREAD_PRIORITY; i++) {
        list_head_init(&thread_ready_list[i]);
    }

    list_head_init(&global_thread_list);
    list_head_init(&thread_suspend_list);

    thread_set_name(&main_thread, "boot_thread");
    thread_set_priority(&main_thread, HIGHEST_THREAD_PRIORITY);
    thread_allocate_id(&main_thread);
    set_cur_thread(&main_thread);
}

static int thread_start_entry(thread_t *thread)
{

}

static void thread_setup_initial_stack(thread_t *thread)
{
    struct context_frame *cf;

    addr_t stack_top = (addr_t)thread->sp_bottom + thread->stack_size;

    // align at 8 bytes
    stack_top = stack_top & (~0x07ul);
    thread->stack_size = (addr_t)thread->sp_bottom - stack_top;

    cf = (struct context_frame*)stack_top;
    cf--;

    arch_init_context_frame(cf, (addr_t*)thread->start_entry, thread->start_arg);

    thread->sp = (addr_t*)cf;
}


static inline void thread_addto_suspend_list(thread_t *thread)
{
    KASSERT(!list_in_list(&thread->node));

    list_add_tail(&thread_suspend_list, &thread->node);
    thread->state = THREAD_SUSPENDED;
}



int thread_suspend(int thread_id)
{
    irqstate_t state;
    bool resched = false;

    if (thread_id_valid(thread_id))
        return -ERR_INVALID_THREAD_ID;

    if (!thread_id == IDLE_THREAD_ID)
        return -ERR_SUSPEDN_IDLE_THREAD;

    thread_t *thread = get_thread_by_id(thread_id);

    state = enter_critical_section();

    if (thread->state == THREAD_RUNNING) {
        resched = true;
    }

    thread_addto_suspend_list(thread);

    leave_critical_section(state);

    if (resched)
        thread_sched();

    return NO_ERR;
}


void thread_yield(void)
{
    irqstate_t state;

    thread_t *old = get_cur_thread();
    KASSERT(old != NULL);

    if (old == idle_thread)
        return;

    state = enter_critical_section();

    old->state = THREAD_READY;
    old->time_remain = 0;
    thread_insert_into_ready_list_tail(old);

    leave_critical_section(state);

    thread_sched();


    return;
}

int thread_resume(int thread_id)
{
    irqstate_t state;
    bool resched = false;

    thread_t *thread = get_thread_by_id(thread_id);

    if (thread == NULL)
        return NO_ERR;

    state = enter_critical_section();

    if (thread->state == THREAD_SUSPENDED) {
        thread->state == THREAD_READY;
        thread_insert_into_ready_list_head(thread);
        resched = true;
    }

    leave_critical_section(state);

    if (resched == true) {
        thread_yield();
    }

    return NO_ERR;
}

int thread_exit(int thread_id)
{

}

int thread_join(int thread_id)
{

}


thread_t* get_new_thread()
{
    thread_t *thread;

    int priority = get_runqueue_first_set_bit();
    KASSERT(!list_is_empty(&thread_ready_list[priority]));

    thread = list_first_entry(&thread_ready_list[priority], struct thread, node);
    list_delete(&thread->node);

    return thread;
}

void thread_sched(void)
{
    irqstate_t state;

    thread_t *old = get_cur_thread();

    state = enter_critical_section();
    old->state = THREAD_READY;
    thread_insert_into_ready_list_tail(old);

    thread_t *new = get_new_thread();
    new->state = THREAD_RUNNING;

    if (old == new) {
        leave_critical_section(state);
        return;
    }

    if (new->time_remain <= 0)
        new->time_remain = new->time_slice;

    set_cur_thread(new);
    arch_context_switch(old->sp, new->sp);

    leave_critical_section(state);
}

int thread_create(const char* name,
                  unsigned int priority,
                  thread_main_t main_entry,
                  void *arg,
                  size_t stack_size,
                  tick_t time_slice,
                  unsigned int flags)
{
    thread_t *thread;
    irqstate_t state;

    thread = malloc(sizeof(*thread));
    if (NULL == thread)
        return -ERR_NO_MEM;

    memset(thread, 0, sizeof(*thread));

    void *stack = malloc(stack_size);
    if (stack == NULL)
        return -ERR_NO_MEM;

    memset(stack, 0, stack_size);

    thread->priority = priority;
    thread->start_entry = thread_start_entry;
    thread->start_arg = thread;
    thread->main_entry = main_entry;
    thread->main_arg = arg;
    thread->sp_bottom = stack;
    thread->stack_size = stack_size;
    thread->time_slice = time_slice;
    thread->flags = flags;

    if (-1 == thread_allocate_id(thread)) {
        return -ERR_NO_THREAD_ID_AVAILABLE;
    }

    thread_setup_initial_stack(thread);

    if (NULL != name) {
        thread_set_name(thread, name);
    }

    state = enter_critical_section();

    thread_addto_suspend_list(thread);

    leave_critical_section(state);

    return thread->thread_id;
}

void thread_sched_start()
{
    thread_t *thread, *temp;
    irqstate_t state;

    state = enter_critical_section();

    list_foreach_entry_safe(&thread_suspend_list, thread, temp, struct thread, node) {
        list_delete(&thread->node);
        thread_insert_into_ready_list_tail(&thread);
    }

    thread = get_new_thread();
    leave_critical_section(state);

    arch_context_switch_to(thread->sp);

    KASSERT(0);
}

