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
    int_nest_cnt--;
}

bool in_nested_interrupt(void)
{
    return (int_nest_cnt > 1)? true: false;
}

