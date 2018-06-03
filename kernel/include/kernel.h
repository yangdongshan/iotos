#ifndef __KERNEL_H
#define __KERNEL_H

#include "mm.h"

#include "thread.h"

//#include "timer.h"

//#include "watchdog.h"

//#include "signal.h"

//#include "mutex.h"

//#include "semphore.h"

//#include "waitqueue.h"

//#include "mailbox.h"


#define enter_critical_section() irq_disable_state_save()

#define leave_critical_section(f) irq_state_restore(f)

#endif // __KERNEL_H


