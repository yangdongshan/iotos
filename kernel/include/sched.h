#ifndef __SCHED_H
#define __SCHED_H

#define OS_RESET            (0x00UL)
#define OS_INIT             (0x01UL)
#define OS_READY            (0x02UL)
#define OS_RUNNING          (0X03UL)
#define OS_STOPPED          (0x04UL)

void os_set_state(unsigned int state);

unsigned int os_get_state(void);

void os_start_sched(void);

#endif //__SCHED_H

