#include <thread.h>
#include <string.h>

#define to_thread_ptr(node) container_of(node, struct list_node, node)

static thread_t thread_node_array[MAX_THREAD_CNT];

static thread_t *thread_id_table[MAX_THREAD_CNT];
static thread_t *cur_thread = NULL;

static thread_t idle_thread_node;
static thread_t idle_thread = NULL;

static struct list_node thread_node_free_list;

static struct list_node thread_ready_list[MAX_THREAD_PRIORITY];

static struct list_node thread_suspend_list;

static const int idle_thread_id = IDLE_THREAD_ID;

// FIXME assume max thread priority is not more than 32
static uint32_t run_queue_bitmap;

static inline thread_t* get_thread_by_id(int thread_id)
{
    KASSERT(thread_id >= 0 && thread_id <= MAX_THREAD_ID);

    return thread_id_table[thread_id];
}

static inline bool thread_id_invalid(int thread_id)
{
    if (thread_id >= 0 && thread_id <= MAX_THREAD_ID)
        return false;
    else
        return true;
}

static void thread_insert_into_ready_list_head(thread_t *thread)
{
    KASSERT(thread->state == THREAD_READY);
    KASSERT(!list_in_list(thread->node));

    list_add_head(&thread_ready_list[thread->priority], &thread->node);
    run_queue_bitmap |= (1<<thread->priority);
}

static void thread_insert_into_ready_list_tail(thread_t *thread)
{
    KASSERT(thread->state == THREAD_READY);
    KASSERT(!list_in_list(thread->node));

    list_add_tail(&thread_ready_list[thread->priority], &thread->node);
    run_queue_bitmap |= (1<<thread->priority);
}

void thread_early_init(void)
{
    unsigned int i;
    thread_t *thread;

    memset(thread_node_array, 0, sizeof(thread_node_array));

    memset(thread_id_table, 0, sizeof(thread_id_table));

    list_head_init(&thread_node_free_list);

    for (i = 0; i < MAX_THREAD_PRIORITY; i++) {
        list_head_init(&thread_ready_list[i]);
    }

    list_head_init(&thread_suspend_list);

    for (i = 0; i < IDLE_THREAD_ID; i++) {
        thread = &thread_node_array[i];
        thread->state = THREAD_UNINITIALIZE;
        thread->thread_id = i;
        list_add_tail(&thread_node_free_list, &thread->node);
        thread_id_table[i] = thread;
    }

    idle_thread = &idle_thread_node;
    set_thread_name(idle, "idle_thread")
    set_cur_thread(idle_thread);
}


static thread_t* get_free_thread_node(void)
{
    irqstate_t state;
    thread_t *thread;
    struct list_node *node;

    if (list_is_empty(&thread_node_free_list))
        return NULL;

    state = enter_critical_section();

    node = list_remove_head(&thread_node_free_list);

    leave_critical_section(state);

    thread = to_thread_ptr(node);

    return thread;
}

static void free_thread_node(thread_t *thread)
{
    irqstate_t state;

    state = enter_critical_section();
    list_add_tail(&thread_node_free_list, &thread->node);
    leave_critical_section(state);
}

static thread_t *get_cur_thread(void)
{
    return cur_thread;
}

static voie set_cur_thread(thread_t *thread)
{
    cur_thread = thread;
}

static int thread_start_entry(thread_t *thread)
{

}

static void thread_setup_initial_stack(thread_t *thread)
{
    struct contex_frame *cf;

    addr_t stack_top = (addr_t)thread->sp_bottom + thread->stack_size;

    // align at 8 bytes
    stack_top = stack_top & (~0x07ul));
    thread->stack_size = (addr_t)thread->sp_bottom - stack_top;

    cf = (struct contex_frame*)stack_top;
    cf--;

    arch_init_context_frame(cf, (addr_t*)thread->start_entry, thread->start_arg);

    thread->sp = (addr_t*)cf;
}


static inline void thread_addto_suspend_list(thread_t *thread)
{
    KASSERT(list_in_list(&thread->node));

    list_add_tail(&thread_suspend_list, &thread->node);
}

static void set_thread_name(thread_t *thread, const char *name)
{
    size_t name_len = strlen(name);
    if (name_len >= MAX_THREAD_NAME_LEN) {
        name_len = MAX_THREAD_NAME_LEN - 1;
        memset(thread->name, 0, MAX_THREAD_NAME_LEN);
        strncpy(thread->name, name, name_len);

    }
}

int thread_suspend(int thread_id)
{
    irqstate_t state;
    bool resched = false;

    if (thread_id_valid(thread_id))
        return -ERR_INVALID_THREAD_ID;

    if (!thread_id == idle_thread_id)
        return -ERR_SUSPEND_IDLE_THREAD;

    state = enter_critical_section();

    thread_t *thread = get_thread_by_id(thread_id);
    KASSERT(thread->state != THREAD_UNINITIALIZE);

    if (thread->state == THREAD_INITIALISZED) {
        thread_addto_suspend_list(thread);
        leave_critical_section(state);
        return NO_ERR;
    }

    if (thread->state == THREAD_RUNNING) {
        resched = true;
        thread_addto_suspend_list(thread);
        thread_resched();
    }

}

int thread_become_ready(int thread_id)
{

}

void thread_yield(void)
{
    irqstate_t state;

    thread_t *old = get_cur_thread();
    KASSERT(old != NULL);

    if (old == idle_thread)
        return;

    state = enter_critical_section();

    thread->state = THREAD_READY;
    thread->time_remain = 0;
    thread_insert_into_ready_list_tail(old);
    thread_resched();

    leave_critical_section(state);

    return;
}

int thread_resume(int thread_id)
{
    irqstate_t state;
    bool resched = false;

    thread_t *thread = get_thread_by_id(thread_id);

    if (thread == idle_thread)
        return NO_ERR;

    state = enter_critical_section();

    if (thread->state == THREAD_INITIALISZED
        || thread->state == THREAD_SUSPENDED) {
        thread->state == THREAD_READY;
        insert_into_run_queue_head(thread);
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

void thread_resched(void)
{
    thread_t *old = get_cur_thread();
    thread_t *new = get_new_thread();

    new->state = THREAD_RUNNING;
    if (old == new)
        return;

    if (new->time_remain <= 0)
        new->time_remain = new->time_slice;

    set_cur_thread(new);

    arch_context_switch(old->sp, new->sp);
}

int thread_create(const char* name,
                  unsigned int priority,
                  thread_main_t main_entry,
                  void *arg,
                  addr_t *stack,
                  size_t stack_size,
                  tick_t time_slice,
                  unsigned int flags)
{
    thread_t *thread;

    thread = get_free_thread_node();
    if (NULL == thread)
        return -ERR_NO_FREE_THREAD_NODE;

    thread->priority = priority;
    thread->start_entry = thread_start_entry;
    thread->start_arg = thread;
    thread->main_entry = main_entry;
    thread->main_arg = arg;
    thread->sp_bottom = stack;
    thread->stack_size = stack_size;
    thread->time_slice = time_slice;
    thread->flags = flags;

    thread_setup_initial_stack(thread);

    if (NULL != name) {
        set_thread_name(thread, name);
    }

    thread->state = THREAD_INITIALISZED;

    return thread->thread_id;
}

