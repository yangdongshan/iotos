#include <timer.h>
#include <time.h>
#include <task.h>
#include <mm.h>
#include <kdebug.h>
#include <irq.h>
#include <err.h>
#include <list.h>
#include <string.h>


#ifndef CONFIG_PRE_ALLOC_TIMER_CNT
#define PRE_ALLOC_TIMER_CNT  (10)
#else
#define PRE_ALLOC_TIMER_CNT CONFIG_PRE_ALLOC_TIMER_CNT
#endif

#define TIMER_ONESHOT       (0x0)
#define TIMER_PERIODICAL    (0x1)
#define TIMER_TYPE_MASK     (0x1)
#define TIMER_TYPE(f)       (f & TIMER_TYPE_MASK)

#define STATIC_ALLOC_TIMER   (0 << 4)
#define DYNAMIC_ALLOC_TIMER  (1 << 4)



static timer_t pre_alloc_timer[PRE_ALLOC_TIMER_CNT];

static list_head_t free_timer_list;
static list_head_t timer_list;
static unsigned int free_timer_cnt;

void timer_init_early(void)
{
    int i;

    list_head_init(&timer_list);

    list_head_init(&free_timer_list);

    free_timer_cnt = 0;

    for (i = 0; i < PRE_ALLOC_TIMER_CNT; i++) {
        pre_alloc_timer[i].flag |= STATIC_ALLOC_TIMER;
        list_add_tail(&free_timer_list, &pre_alloc_timer[i].node);
        free_timer_cnt++;
    }
}

static timer_t *get_free_timer(void)
{
    timer_t *timer;
    irqstate_t state;

    if (list_is_empty(&free_timer_list)) {
#ifdef CONFIG_DYNAMIC_ALLOC_TIMER
        KDBG("malloc new timer\r\n");
        timer = (timer_t*)malloc(sizeof(timer_t));
        timer->flag |= DYNAMIC_ALLOC_TIMER;
        return timer;
#else
        return NULL;
#endif
    } else {
        state = enter_critical_section();

        timer = list_first_entry(&free_timer_list, timer_t, node);
        list_delete(&timer->node);
        free_timer_cnt--;
        KDBG("get free timer from freelist, free cnt %d\r\n",
                free_timer_cnt);
        leave_critical_section(state);
        return timer;
    }
}

static void free_timer(timer_t *timer)
{
    if ((timer->flag & DYNAMIC_ALLOC_TIMER) &&
        (free_timer_cnt >= PRE_ALLOC_TIMER_CNT)) {
        KDBG("free timer %s to memory, free cnt %d\r\n",
                timer->name, free_timer_cnt);
        free(timer);
    } else {
        irqstate_t state;

        state = enter_critical_section();
        list_add_tail(&free_timer_list, &timer->node);
        free_timer_cnt++;
        KDBG("free timer %s to freelist, free cnt %d\r\n",
                timer->name, free_timer_cnt);
        leave_critical_section(state);
    }
}

static timer_t* register_timer(char *name,
                          unsigned int delay,
                          timeout_cb handle,
                          void *arg,
                          int type)
{
    if (in_nested_interrupt())
        return ERR_IN_INTERRUPT;

    KASSERT(handle != NULL);

    timer_t *timer = get_free_timer();
    KASSERT(timer != NULL);

    tick_t cur_tick = get_sys_tick();
    timer->cycle = MS2TICKS(delay);
    timer->timeout = cur_tick + timer->cycle;
    timer->timeout_handle = handle;
    timer->arg = arg;
    timer->flag |= type;
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
            if (iter_timer->timeout > timer->timeout) {
                list_add_before(&iter_timer->node, &timer->node);
                inserted = true;
                break;
            }
        }
        if (inserted == false)
            list_add_tail(&timer_list, &timer->node);
    }

    leave_critical_section(state);

    KDBG("registered %s timer %s on time list %p,\n"
         "tick period 0x%x, timeout 0x%x\r\n",
         (timer->flag & TIMER_PERIODICAL)?
         "periodical":"oneshot", timer->name,
         &timer_list, timer->cycle, timer->timeout);

    return timer;
}

timer_t*  register_oneshot_timer(char *name, unsigned int delay, timeout_cb handle, void *arg)
{
    return register_timer(name, delay, handle, arg, TIMER_ONESHOT);
}

timer_t* register_periodical_timer(char *name, unsigned int delay, timeout_cb handle, void *arg)
{
    return register_timer(name, delay, handle, arg, TIMER_PERIODICAL);
}

int cancel_timer(timer_t *timer)
{
    int ret = -1;
    struct list_node *cur_node, *next_node;
    timer_t *iter_timer;

    KASSERT(timer != NULL);
    KASSERT(!list_is_empty(&timer_list));

    next_node = timer_list.next;
    while (next_node != &timer_list) {
        cur_node = next_node;
        next_node = cur_node->next;
        iter_timer = list_entry(cur_node, timer_t, node);
        if (iter_timer == timer) {
            list_delete(&iter_timer->node);
            free_timer(iter_timer);
            ret = 0;
            break;
        }
    }

    return ret;
}

void timer_tick(void)
{
    irqstate_t state;
    tick_t cur_tick;
    bool sched = false;

    cur_tick = get_sys_tick();
    state = enter_critical_section();

    if (!list_is_empty(&timer_list)) {
        struct list_node *iter_node, *next_node;
        iter_node = timer_list.next;
        while (iter_node != &timer_list) {
            timer_t *iter_timer;
            next_node = iter_node->next;
            iter_timer = list_entry(iter_node, timer_t, node);
            if (cur_tick >= iter_timer->timeout) {
                KDBG("cur_tick 0x%x, timer %s timeout tick 0x%x\r\n",
                     cur_tick, iter_timer->name, iter_timer->timeout);
                iter_timer->timeout_handle(iter_timer->arg);
                if ((iter_timer->flag & TIMER_TYPE_MASK) == TIMER_ONESHOT) {
                    list_delete(&iter_timer->node);
                    free_timer(iter_timer);
                } else {
                    iter_timer->timeout = cur_tick + iter_timer->cycle;
                }
            }
            iter_node = next_node;
        }
    }

    task_t *task = get_cur_task();
    if (task->time_remain > 0) {
        if (--task->time_remain == 0) {
            task->time_remain = task->time_slice;
            sched = true;
        }
    }

    // FIXME: or high priority task is ready
    if (task_can_be_preempted() || (sched == true)) {
        task_become_ready_tail(task);
        task_sched();
    }

    leave_critical_section(state);

}
