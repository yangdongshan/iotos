#include <string.h>
#include <err.h>
#include <irq.h>
#include <kernel.h>
#include <kdebug.h>


#define to_thread_ptr(node) container_of(node, struct list_node, node)

#define BITS_U8_CNT(bits) ((bits + 7)/8)

static thread_t *thread_id_table[MAX_THREAD_CNT];
static thread_t *cur_thread = NULL;

static int idle_thread_id = -1;

static struct list_node global_thread_list;
static struct list_node thread_ready_list[LOWEST_THREAD_PRIORITY + 1];

static struct list_node thread_suspend_list;

// FIXME assume max thread priority is not more than 32
#define RUNQUEUE_SIZE BITS_U8_CNT(LOWEST_THREAD_PRIORITY + 1)
static uint8_t runqueue_bitmap[RUNQUEUE_SIZE];

static int get_runqueue_first_set_bit(void)
{
    int byte;
    int bit = 0;
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
    KASSERT((priority >= 0) && (priority <= LOWEST_THREAD_PRIORITY));

    uint8_t byte = priority / 8;
    uint8_t bit = priority % 8;

    KDBG(DEBUG, "set runqueue byte %d bit %d for priority %d\r\n", byte, bit, priority);
    runqueue_bitmap[byte] |= 1 << bit;
}

static void clear_runqueue_bit(int priority)
{
    KASSERT((priority >= 0) && (priority <= LOWEST_THREAD_PRIORITY));

    uint8_t byte = priority / 8;
    uint8_t bit = priority % 8;

    runqueue_bitmap[byte] &= ~(1 << bit);

    KDBG(DEBUG, "clear runqueue byte %d bit %d for priority %d\r\n", byte, bit, priority);

}
static inline int get_idle_thread_id(void)
{
    return idle_thread_id;
}

void set_idle_thread_id(int id)
{
    idle_thread_id = id;
}

static inline thread_t* idle_thread(void)
{
    return thread_id_table[idle_thread_id];
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
    if (((thread_id >= 0) && (thread_id <= MAX_THREAD_ID))
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
    }

    memset(thread->name, 0, MAX_THREAD_NAME_LEN);

    state = enter_critical_section();
    strncpy(thread->name, name, name_len);
    kdebug_print("%s thread->name %s\r\n", __func__, name);
    leave_critical_section(state);

}

static void thread_set_priority(thread_t *thread, int priority)
{
    irqstate_t state;

    state = enter_critical_section();

    thread->priority = priority;

    leave_critical_section(state);
}

void thread_insert_into_ready_list_head(thread_t *thread)
{

    KASSERT(thread->state == THREAD_READY);
    KASSERT(!list_in_list(&thread->node));

    list_add_head(&thread_ready_list[thread->priority], &thread->node);
    set_runqueue_bit(thread->priority);
}

static void thread_insert_into_ready_list_tail(thread_t *thread)
{

    KASSERT(thread->state == THREAD_READY);
    KASSERT(!list_in_list(&thread->node));

    KDBG(DEBUG, "insert old thread %s into ready list\r\n", thread->name);
    list_add_tail(&thread_ready_list[thread->priority], &thread->node);
    set_runqueue_bit(thread->priority);
}

thread_t *get_cur_thread(void)
{
    return cur_thread;
}

static inline void set_cur_thread(thread_t *thread)
{
    irqstate_t state;

    state = enter_critical_section();
    KDBG(DEBUG, "set cur_thread %s\r\n", thread->name);
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




void thread_init(void)
{
    unsigned int i;

    memset(thread_id_table, 0, sizeof(thread_id_table));
    memset(runqueue_bitmap, 0, sizeof(runqueue_bitmap));

    for (i = 0; i <= LOWEST_THREAD_PRIORITY; i++) {
        list_head_init(&thread_ready_list[i]);
    }

    list_head_init(&global_thread_list);
    list_head_init(&thread_suspend_list);


}

static int start_entry(void *arg)
{
    int ret = NO_ERR;
    thread_t *thread = (thread_t*)arg;

    KDBG(INFO, "thread name %s\r\n", thread->name);
    if (thread->main_entry) {
        ret = thread->main_entry(thread->main_arg);
    }

    return ret;
}

static void thread_setup_initial_stack(thread_t *thread)
{
    struct context_frame *cf;

    addr_t stack_top = (addr_t)thread->sp_alloc_addr + thread->stack_size;

    // align at 8 bytes
    stack_top = stack_top & (~0x07ul);
    thread->stack_size = stack_top - (addr_t)thread->sp_alloc_addr;

    cf = (struct context_frame*)stack_top;
    cf--;

    arch_init_context_frame(cf, (addr_t*)thread->start_entry, thread->start_arg);

    thread->sp = (addr_t)cf;
}


static inline void thread_addto_suspend_list(thread_t *thread)
{
    KASSERT(!list_in_list(&thread->node));

    list_add_tail(&thread_suspend_list, &thread->node);
    thread->state = THREAD_SUSPENDED;
}


void thread_become_ready(thread_t *thread)
{
    irqstate_t state;

    KDBG(DEBUG, "thread %s become ready\r\n", thread->name);

    KASSERT(thread->state != THREAD_READY);
    KASSERT(!list_in_list(&thread->node));

    state = enter_critical_section();

    thread->state = THREAD_READY;
    thread->time_remain = thread->time_slice;
    thread_insert_into_ready_list_tail(thread);

    KDBG(DEBUG, "thread %s insert into ready list done\r\n", thread->name);

    leave_critical_section(state);
}

int thread_suspend(int thread_id)
{
    irqstate_t state;
    bool resched = false;

    if (thread_id_valid(thread_id))
        return -ERR_INVALID_THREAD_ID;

    if (thread_id == idle_thread_id)
        return -ERR_SUSPEDN_IDLE_THREAD;

    thread_t *thread = get_thread_by_id(thread_id);

    state = enter_critical_section();

    if (thread->state == THREAD_RUNNING) {
        resched = true;
    }

    thread_addto_suspend_list(thread);

    if (resched)
        thread_sched();

    leave_critical_section(state);


    return NO_ERR;
}


void thread_yield(void)
{
    irqstate_t state;
    thread_t *thread;

    state = enter_critical_section();

    thread= get_cur_thread();
    thread->state = THREAD_READY;
    thread_insert_into_ready_list_tail(thread);
    thread_sched();

    leave_critical_section(state);

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
    return NO_ERR;
}

int thread_join(int thread_id)
{
    return NO_ERR;
}


thread_t* get_new_thread()
{
    thread_t *thread;

    int priority = get_runqueue_first_set_bit();

    // no thread other than idle is ready
    if (priority == -1) {
        return NULL;
    }

    KASSERT(!list_is_empty(&thread_ready_list[priority]));

    thread = list_first_entry(&thread_ready_list[priority], struct thread, node);

    list_delete(&thread->node);
    if (list_is_empty(&thread_ready_list[priority])) {
        clear_runqueue_bit(priority);
    }

    return thread;
}


// should be invoked with interrupt disabled
void thread_sched(void)
{
    thread_t *old = get_cur_thread();
    thread_t *new = get_new_thread();

    if (new == NULL) {
        KDBG(DEBUG, "no more thread is ready\r\n");
        return;
    }

    KDBG(DEBUG, "%s, old thread %s, time_remain 0x%x, new thread %s, time_remain 0x%x\r\n",
            __func__, old->name, old->time_remain,  new->name, new->time_remain);

    new->state = THREAD_RUNNING;

    set_cur_thread(new);
    arch_context_switch((unsigned char*)&new->sp, (unsigned char*)&old->sp);
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
    int ret = NO_ERR;

    if (main_entry == NULL) {
        return -ERR_INVALID_THREAD_ENTRY;
    }

    thread = malloc(sizeof(*thread));
    if (NULL == thread)
        return -ERR_NO_MEM;

    memset(thread, 0, sizeof(*thread));

    void *stack = malloc(stack_size);
    if (stack == NULL) {
        ret = -ERR_NO_MEM;
        goto m_free1;
    }

    memset(stack, 0, stack_size);

    thread->priority = priority;
    thread->start_entry = start_entry;
    thread->start_arg = thread;
    thread->main_entry = main_entry;
    thread->main_arg = arg;
    thread->sp_alloc_addr = stack;
    thread->stack_size = stack_size;
    if (time_slice <= 0)
        time_slice = THREAD_DEFAULT_TIME_SLICE;
    thread->time_slice = time_slice;
    thread->time_remain = time_slice;
    thread->flags = flags;

    if (-1 == thread_allocate_id(thread)) {
        ret = -ERR_NO_THREAD_ID_AVAILABLE;
        goto m_free2;
    }

    thread_setup_initial_stack(thread);
    KDBG(DEBUG, "thread %p stack alloc addr 0x%x, stack size 0x%x, sp 0x%x\r\n", thread, thread->sp_alloc_addr, thread->stack_size, thread->sp);
    if (NULL != name) {
        thread_set_name(thread, name);
    }

    KDBG(DEBUG, "%s create thread %p, name %s, time_remain 0x%x\r\n",
            __func__, thread, thread->name, thread->time_remain);
    state = enter_critical_section();

    thread_addto_suspend_list(thread);

    leave_critical_section(state);

    return thread->thread_id;

m_free2:
    free(stack);
m_free1:
    free(thread);
    return ret;
}


void thread_sched_start(void)
{
    thread_t *thread, *temp;
    irqstate_t state;

    state = enter_critical_section();

    list_foreach_entry_safe(&thread_suspend_list, thread, temp, struct thread, node) {
            kdebug_print("insert thread %p name %s to ready list\r\n", thread, thread->name);
            list_delete(&thread->node);
            thread->state = THREAD_READY;
            kdebug_print("thread->node prev %p, next %p, priority %d\r\n",
                    thread->node.prev, thread->node.next,
                    thread->priority);
            thread_insert_into_ready_list_tail(thread);
    }
    thread = get_new_thread();
    kdebug_print("get new thread %s %p from ready list\r\n",  thread->name, thread);
    set_cur_thread(thread);
    leave_critical_section(state);

    arch_context_switch_to((unsigned char*)&thread->sp);

    kdebug_print("shouldn't return here\r\n");
    KASSERT(0);
}

