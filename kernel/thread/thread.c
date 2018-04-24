#include <kernel_cfg.h>
#include <thread.h>
#include <string.h>

static thread_t thread_node_array[MAX_THREAD_CNT];

static thread_t *thread_id_table[MAX_THREAD_CNT];

static struct list_node thread_node_free_list;

void thread_early_init(void)
{
    unsigned int i;
    thread_t *thread;

    memset(thread_node_array, 0, sizeof(thread_node_array));

    memset(thread_id_table, 0, sizeof(thid_table));

    list_head_init(&thread_node_free_list);

    for (i = 0; i < MAX_THREAD_CNT; i++) {
        thread = &thread_node_array[i];
        thread->thread_id = i;
        list_add_tail(&thread_node_free_list, &thread->node);
        thread_id_table[i] = thread;
    }
}

#define to_thread_ptr(node) container_of(node, struct list_node, node)

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

static int thread_original_entry(thread_t *thread)
{

}

static void thread_setup_initial_stack(thread_t *thread)
{
    struct contex_frame *cf;

    addr_t stack_top = (addr_t)thread->sp_bottom + thread->stack_size;

    // align at 4 bytes
    stack_top = stack_top & (~0x03ul));

    cf = (struct contex_frame*)stack_top;
    cf--;

    arch_init_context_frame(cf, thread->thread_entry);

    thread->sp = (addr_t*)cf;
}

int thread_create(const char* name, unsigned int priority, thread_main_t entry, void *arg, addr_t *stack, size_t stack_size)
{
    thread_t *thread;

    thread = get_free_thread_node();
    if (NULL == thread)
        return ERR_NO_FREE_THREAD_NODE;

    thread->priority = priority;
    thread->thread_entry = thread_original_entry;
    thread->thread_main = entry;
    thread->arg = arg;
    thread->sp_bottom = stack;
    thread->stack_size = stack_size;

    thread_setup_initial_stack(thread);

    thread->state = THREAD_SUSPENDED;

}
