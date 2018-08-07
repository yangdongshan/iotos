#include <timer.h>
#include <time.h>
#include <thread.h>
#include <mm.h>
#include <kdebug.h>
#include <irq.h>
#include <err.h>
#include <list.h>
#include <string.h>


#ifndef CONFIG_PREALLOC_TIMER_CNT
#define PREALLOC_TIMER_CNT  (10)
#else
#define PREALLOC_TIMER_CNT CONFIG_PREALLOC_TIMER_CNT
#endif

#define TIMER_ONESHOT       (0x0)
#define TIMER_PERIODICAL    (0x1)
#define TIMER_TYPE_MASK     (0x1)
#define TIMER_TYPE(f)       (f & TIMER_TYPE_MASK)

typedef int (*timeout_cb)(void *arg);

typedef struct timer {
    struct list_node node;
    unsigned int timeout;
    unsigned int cycle;
    timeout_cb timeout_handle;
    void *arg;
    int flags;
    char *name;
} timer_t;

static list_head_t free_timer_list;
static list_head_t timer_list;
static unsigned int registered_timer_cnt;
static unsigned int free_timer_cnt;

void timer_init(void)
{
   list_head_init(&timer_list);

   list_head_init(&free_timer_list);
    free_timer_cnt = 0;

   timer_t *timer = (timer_t*) malloc(PREALLOC_TIMER_CNT * sizeof(timer_t));

   if (timer != NULL) {
        for (int i = 0; i < PREALLOC_TIMER_CNT; i++) {
            list_add_tail(&free_timer_list, &timer->node);
            timer++;
            free_timer_cnt++;
        }
   }

   registered_timer_cnt = 0;
}

static timer_t *get_free_timer(void)
{
    if (list_is_empty(&free_timer_list)) {
        KDBG(DEBUG, "malloc new timer\r\n");
        return (timer_t*)malloc(sizeof(timer_t));
    } else {
        timer_t *timer;
        irqstate_t state;
        state = enter_critical_section();

        timer = list_first_entry(&free_timer_list, timer_t, node);
        list_delete(&timer->node);
        free_timer_cnt--;
        KDBG(DEBUG, "get free timer from freelist, free cnt %d\r\n",
                free_timer_cnt);
        leave_critical_section(state);
        return timer;
    }
}

static void free_timer(timer_t *timer)
{
    if (free_timer_cnt >= PREALLOC_TIMER_CNT) {
        KDBG(DEBUG, "free timer %s to memory, free cnt %d\r\n",
                timer->name, free_timer_cnt);
        mm_free(timer);
    } else {
        irqstate_t state;

        state = enter_critical_section();
        list_add_tail(&free_timer_list, &timer->node);
        free_timer_cnt++;
        KDBG(DEBUG, "free timer %s to freelist, free cnt %d\r\n",
                timer->name, free_timer_cnt);
        leave_critical_section(state);
    }
}

static int register_timer(char *name,
                          unsigned int delay,
                          timeout_cb handle,
                          void *arg,
                          int flags)
{
    if (in_nested_interrupt())
        return ERR_IN_INTERRUPT;

    if (handle == NULL)
        return ERR_NULL_PTR;

    if (delay == 0) {
        return ERR_INVALID_ARG;
    }

    timer_t *timer = get_free_timer();
    if (timer == NULL) {
        return ERR_NO_MEM;
    }
    memset(timer, 0, sizeof(timer_t));

    tick_t cur_tick = get_sys_tick();
    timer->cycle = MS2TICKS(delay);
    timer->timeout = cur_tick + timer->cycle;
    timer->timeout_handle = handle;
    timer->arg = arg;
    timer->flags = flags;
    timer->name = name;

    irqstate_t state;
    state = enter_critical_section();

    if (list_is_empty(&timer_list)) {
        list_add_head(&timer_list, &timer->node);
    } else {
        struct list_node *iter_node, *temp_node;
        timer_t *iter_timer;
        bool inserted = false;
        list_foreach_safe(&timer_list, iter_node, temp_node) {
            iter_timer = list_entry(iter_node, timer_t, node);
            if (iter_timer->timeout >= timer->timeout) {
                list_add_before(&iter_timer->node, &timer->node);
                inserted = true;
                break;
            }
        }
        if (inserted == false)
            list_add_tail(&timer_list, &timer->node);
    }

    leave_critical_section(state);

    KDBG(DEBUG, "registered %s timer %s on time list %p, tick period 0x%x, timeout 0x%x\r\n",
            ((timer->flags & TIMER_TYPE_MASK)== TIMER_ONESHOT)?
            "oneshot":"periodical", timer->name, &timer_list, timer->cycle, timer->timeout);
    return NO_ERR;
}

int register_oneshot_timer(char *name, unsigned int delay, timeout_cb handle, void *arg)
{
    return register_timer(name, delay, handle, arg, TIMER_ONESHOT);
}

int register_periodical_timer(char *name, unsigned int delay, timeout_cb handle, void *arg)
{
    return register_timer(name, delay, handle, arg, TIMER_PERIODICAL);
}

void timer_tick(void)
{
    irqstate_t state;
    tick_t cur_tick;
    bool sched = false;

    if (list_is_empty(&timer_list)) {
        return;
    }

    struct list_node *iter_node, *next_node;
    list_head_t temp_list_head;

    list_head_init(&temp_list_head);
    cur_tick = get_sys_tick();
    state = enter_critical_section();

    list_foreach(&timer_list, iter_node) {
        timer_t *iter_timer;
        iter_timer = list_entry(iter_node, timer_t, node);
       if (cur_tick >= iter_timer->timeout) {
            KDBG(DEBUG, "cur_tick 0x%x, timer %s timeout tick 0x%x\r\n",
                cur_tick, iter_timer->name, iter_timer->timeout);
            next_node = iter_node->next;

            leave_critical_section(state);
            iter_timer->timeout_handle(iter_timer->arg);
            state = enter_critical_section();

            list_delete(&iter_timer->node);
            if ((iter_timer->flags & TIMER_TYPE_MASK) == TIMER_ONESHOT) {
                iter_node = next_node->next;
                free_timer(iter_timer);
            } else {
                iter_timer->timeout = cur_tick + iter_timer->cycle;
                if (next_node == &timer_list) {
                    list_add_tail(&timer_list, &iter_timer->node);
                } else {
                    struct list_node *temp_node = next_node;
                    for (; temp_node != &timer_list; temp_node = temp_node->next) {
                        timer_t *temp_timer;
                        temp_timer = list_entry(temp_node, timer_t, node);
                        if (temp_timer->timeout > iter_timer->timeout) {
                            list_add_before(&temp_timer->node, &iter_timer->node);
                        } else {
                            list_add_after(&temp_timer->node, &iter_timer->node);
                        }
                    }
                    if (iter_node != next_node->prev) {
                        iter_node =next_node->prev;
                    } else {
                        iter_node = next_node;
                    }
                }
            }
       } else {
           // break;
       }
    }

    thread_t *thread = get_cur_thread();
    if (thread->time_remain > 0) {
        if (--thread->time_remain == 0) {
            thread_become_ready(thread);
            sched = true;
        }
    }

    // FIXME: or high priority thread is ready
    if (thread_can_be_preempted() || (sched == true)) {
        thread_sched();
    }

    leave_critical_section(state);


}
