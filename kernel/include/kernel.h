#ifndef __KERNEL_H
#define __KERNEL_H

#include "mm.h"

#include "task.h"

#include "timer.h"

//#include "watchdog.h"

//#include "signal.h"

//#include "mutex.h"

#include "semphore.h"

//#include "waitqueue.h"

//#include "mailbox.h"

void os_init(void);

void os_run(void);

#endif // __KERNEL_H


