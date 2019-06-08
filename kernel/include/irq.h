#ifndef __CORE_H
#define __CORE_H

#include "port.h"

#define INTERRUPT_NEST_MAX  ((unsigned long)-1)

#define enter_critical_section() arch_enter_critical_section()

#define leave_critical_section(f) arch_leave_critical_section(f)

#define intrpt_disable() arch_intrpt_disable()

#define intrpt_enable() arch_intrpt_enable()

void interrupt_enter(void);

void interrupt_leave(void);

int is_interrupt_nested(void);

int is_in_interrupt(void);

#endif // __CORE_H

