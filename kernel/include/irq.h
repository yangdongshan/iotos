#ifndef IRQ_H
#define IRQ_H

#include "port.h"

#define enter_critical_section() irq_disable_state_save()

#define leave_critical_section(f) irq_state_restore(f)

extern int int_nest_cnt;

static inline int enter_interrupt(void)
{
    int_nest_cnt++;

    return int_nest_cnt;
}

static inline int leave_interrupt(void)
{
    int_nest_cnt--;

    return int_nest_cnt;
}

bool in_nested_interrupt(void);

#endif // IRQ_H

