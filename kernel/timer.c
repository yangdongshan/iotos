#include <timer.h>
#include <time.h>
#include <thread.h>
#include <mm.h>
#include <kdebug.h>
#include <irq.h>
#include <err.h>
#include <list.h>
#include <string.h>

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
} timer_t;


static list_head_t timer_list;
static unsigned int registered_timer_cnt;

void timer_init(void)
{
   list_head_init(&timer_list);

   registered_timer_cnt = 0;
}



static int register_timer(unsigned int delay,
                          timeout_cb handle,
                          void *arg,
                          int flags)
{
    if (in_interrupt())
        return ERR_IN_INTERRUPT;

    if (handle == NULL)
        return ERR_NULL_PTR;

    if (delay == 0) {
        return ERR_INVALID_ARG;
    }

    timer_t *timer = (timer_t*)mm_malloc(sizeof(timer_t));
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

    irqstate_t state;
    state = enter_critical_section();

    if (list_is_empty(&timer_list)) {
        list_add_head(&timer_list, &timer->node);
    } else {
        struct list_node *iter_node, *temp_node;
        timer_t *iter_timer;
        list_foreach_safe(&timer_list, iter_node, temp_node) {
            iter_timer = list_entry(iter_node, timer_t, node);
            if (iter_timer->timeout >= timer->timeout) {
                list_add_before(&iter_timer->node, &timer->node);
                break;
            }
        }
    }

    leave_critical_section(state);

    KDBG(DEBUG, "registered %s timer %p on time list %p, tick period %d\r\n",
            ((timer->flags & TIMER_TYPE_MASK)== TIMER_ONESHOT)? "oneshot":"periodical", timer, &timer_list, timer->cycle);
    return NO_ERR;
}

int register_oneshot_timer(unsigned int delay, timeout_cb handle, void *arg)
{
    return register_timer(delay, handle, arg, TIMER_ONESHOT);
}

int register_periodical_timer(unsigned int delay, timeout_cb handle, void *arg)
{
    return register_timer(delay, handle, arg, TIMER_PERIODICAL);
}

void timer_tick(void)
{
    irqstate_t state;
    tick_t cur_tick;
    bool sched = false;

    if (list_is_empty(&timer_list)) {
        return;
    }

    struct list_node *iter_node, *temp_node;
    cur_tick = get_sys_tick();
    state = enter_critical_section();

    list_foreach_safe(&timer_list, iter_node, temp_node) {
        timer_t *iter_timer;
        iter_timer = list_entry(iter_node, timer_t, node);
       if (cur_tick >= iter_timer->timeout) {
            list_delete(&iter_timer->node);

            leave_critical_section(state);
            iter_timer->timeout_handle(iter_timer->arg);
            state = enter_critical_section();

            if ((iter_timer->flags & TIMER_TYPE_MASK) == TIMER_ONESHOT) {
                mm_free(iter_timer);
            } else {
                iter_timer->timeout = cur_tick + iter_timer->cycle;
                if (temp_node == &timer_list) {
                    list_add_tail(&timer_list, &iter_timer->node);
                } else {
                    struct list_node *node = temp_node;
                    for (; node != &timer_list; node = node->next) {
                        timer_t *next_timer;
                        next_timer = list_entry(node, timer_t, node);
                        if (next_timer->timeout > iter_timer->timeout) {
                            list_add_before(&next_timer->node, &iter_timer->node);
                        } else {
                            list_add_after(&next_timer->node, &iter_timer->node);
                        }
                    }
                }
            }

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
    if (sched == true) {
        thread_sched();
    }

    leave_critical_section(state);


}
