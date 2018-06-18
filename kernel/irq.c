#include "irq.h"

int int_nest_cnt = 0;

bool in_interrupt(void)
{
    return (int_nest_cnt > 0)? true: false;
}

