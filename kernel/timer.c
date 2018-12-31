#include <timer.h>
#include <time.h>
#include <task.h>
#include <sched.h>
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
#define TIMER_IS_ONESHOT(t)  (TIMER_TYPE(t->flag) == TIMER_ONESHOT)
#define TIMER_IS_PERIODICAL(t) (!(TIMER_IS_ONESHOT(t)))

#define STATIC_ALLOC_TIMER   (0 << 4)
#define DYNAMIC_ALLOC_TIMER  (1 << 4)


static list_head_t timer_list;

void timer_init_early(void)
{
    list_head_init(&timer_list);
}

static void timer_list_insert(timer_t *timer)
{
    timer_t *iter;
    int inserted = 0;

    if (list_is_empty(&timer_list)) {
        list_add_head(&timer_list, &timer->node);
    } else {
        list_foreach_entry(&timer_list, iter, timer_t, node) {
            if (timer->timeout < iter->timeout) {
                list_add_before(&iter->node, &timer->node);
                inserted = 1;
                break;
            }
        }

        if (inserted == 0) {
            list_add_tail(&timer_list, &timer->node);
        }
    }
}

static int register_timer(timer_t *timer,
                          const char *name,
                          unsigned long delay_ticks,
                          timeout_cb handle,
                          void *arg,
                          int type)
{
    irqstate_t state;
    tick_t now;

    if (in_nested_interrupt())
        return ERR_INT_NESTED;

    KASSERT(handle != NULL);
    KASSERT(timer != NULL);

    now = get_sys_tick();
    timer->cycle = delay_ticks;
    timer->timeout = now + timer->cycle;
    timer->timeout_handle = handle;
    timer->arg = arg;
    timer->flag |= type;
    timer->name = name;

    state = enter_critical_section();

    timer_list_insert(timer);

    leave_critical_section(state);

    KDBG("registered %s timer %s on time list %p,\n"
         "tick period 0x%x, timeout 0x%x\r\n",
         (timer->flag & TIMER_PERIODICAL)?
         "periodical":"oneshot", timer->name,
         &timer_list, timer->cycle, timer->timeout);

    return ERR_OK;
}

int register_oneshot_timer(timer_t *timer, const char *name, unsigned long delay_ticks, timeout_cb handle, void *arg)
{
    return register_timer(timer, name, delay_ticks, handle, arg, TIMER_ONESHOT);
}

int register_periodical_timer(timer_t *timer, const char *name, unsigned long delay_ticks, timeout_cb handle, void *arg)
{
    return register_timer(timer, name, delay_ticks, handle, arg, TIMER_PERIODICAL);
}

int cancel_timer(timer_t *timer)
{
    timer_t *iter;
    int ret = -1;

    KASSERT(timer != NULL);
    KASSERT(!list_is_empty(&timer_list));
    KASSERT(!list_in_list(&timer->node));

    list_foreach_entry(&timer_list, iter, timer_t, node) {
        if (timer == iter) {
            list_delete(&iter->node);
            ret = 0;
            break;
        }
    }

    return ret;
}

void timer_tick(void)
{
    irqstate_t state;
    task_t *cur;
    timer_t *iter;
    tick_t tick, now;

    if (OS_RUNNING != os_get_state())
        return;

    state = enter_critical_section();

    if (list_is_empty(&timer_list))
        goto dec_remain;

    tick = get_sys_tick();
    now = tick;
    for (;;) {
        iter = list_first_entry(&timer_list, timer_t, node);
        if (iter->timeout <= now) {
            KDBG("cur_tick 0x%x, timer %s timeout tick 0x%x\r\n",
                 now, iter->name, iter->timeout);
            iter->timeout_handle(iter->arg);
            list_delete(&iter->node);
            if (TIMER_IS_PERIODICAL(iter)) {
                iter->timeout = tick + iter->cycle;
                timer_list_insert(iter);
            }
        } else {
            break;
        }

        now = get_sys_tick();
    }

dec_remain:
    cur = get_cur_task();
    if (--cur->time_remain == 0) {
        TASK_RESCHED_SET(cur);
    }

    leave_critical_section(state);
}

