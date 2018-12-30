#include "irq.h"
#include "task.h"

int int_nest_cnt = 0;

void enter_interrupt(void)
{
    int_nest_cnt++;
}

// TODO: sched task if preepmt is enabled
void leave_interrupt(void)
{
    task_t *cur;
    irqstate_t state;

    int_nest_cnt--;

    state = enter_critical_section();

    cur = get_cur_task();
    if (cur && (task_can_be_preempted() || TASK_NEED_RESCHED(cur))) {
        task_become_ready_tail(cur);
        task_switch();
    }

    leave_critical_section(state);
}

bool in_nested_interrupt(void)
{
    return (int_nest_cnt > 1)? true: false;
}

