#ifndef __CORE_H
#define __CORE_H

#include "port.h"

#define AX_TRUE     (0x01)
#define AX_FALSE    (0x00)

#define AX_WAIT_FOREVER     ((tick_t)-1)
#define AX_WAIT_NONE        ((tick_t)0)

#define INTERRUPT_NEST_MAX  ((unsigned long)-1)

#define enter_critical_section() arch_enter_critical_section()

#define leave_critical_section(f) arch_leave_critical_section(f)

void interrupt_enter(void);

void interrupt_leave(void);

bool is_interrupt_nested(void);

bool is_in_interrupt(void);

#endif // __CORE_H

