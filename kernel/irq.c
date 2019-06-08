#include "irq.h"
#include "sched.h"
#include "task.h"
#include <kdebug.h>

unsigned long interrupt_nest_cnt = 0;

unsigned long get_interrupt_nest_cnt(void)
{
    return interrupt_nest_cnt;
}

unsigned long interrupt_nest_cnt_inc(void)
{
	KASSERT(interrupt_nest_cnt < INTERRUPT_NEST_MAX);

    interrupt_nest_cnt++;
	return interrupt_nest_cnt;
}

unsigned long interrupt_nest_cnt_dec(void)
{
	KASSERT(interrupt_nest_cnt > 0);

    interrupt_nest_cnt--;

	return interrupt_nest_cnt;
}

void interrupt_enter(void)
{
    irqstate_t state;

    state = enter_critical_section();
    interrupt_nest_cnt_inc();
    leave_critical_section(state);
}

// TODO: sched task if preepmt is enabled
void interrupt_leave(void)
{
    task_t *cur;
    irqstate_t state;

	int resched = 0;

    state = enter_critical_section();

    if (sched_get_state() != SCHED_RUNNING)
        goto out;

    if (interrupt_nest_cnt_dec() > 0) {
        goto out;
    }

	if (IS_SCHED_LOCKED()) {
		goto out;
	}

	cur = get_cur_task();

	/* if the task isn't idle task, reduce it's remaining time */
	if (cur->flags & TF_IDLE_TASK == 0) {
		cur->time_remain--;
	}

	if (cur->time_remain == 0 || task_need_resched()) {
		list_delete(&cur->node);
		cur->time_remain = cur->time_slice;
		cur->state = TS_READY;
		task_addto_ready_list_tail(cur);
		resched = 1;
	}

	if (resched == 1) {
		task_int_switch();
	}

out:
    leave_critical_section(state);
}

int is_interrupt_nested(void)
{
    return (interrupt_nest_cnt > 1)? 1: 0;
}

int is_in_interrupt(void)
{
    return (interrupt_nest_cnt > 0)? 1: 0;
}

