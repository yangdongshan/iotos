#ifndef IRQ_H
#define IRQ_H

#include "port.h"

#define enter_critical_section() irq_disable_state_save()

#define leave_critical_section(f) irq_state_restore(f)


void enter_interrupt(void);

void leave_interrupt(void);

bool in_nested_interrupt(void);

#endif // IRQ_H

