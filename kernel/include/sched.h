#ifndef __SCHED_H
#define __SCHED_H

#define SCHED_RESET            (0x00UL)
#define SCHED_INIT             (0x01UL)
#define SCHED_READY            (0x02UL)
#define SCHED_RUNNING          (0X03UL)
#define SCHED_STOPPED          (0x04UL)

#define SCHED_LOCK_NEST_MAX ((unsigned int)(-1))

void sched_set_state(unsigned int state);

unsigned int sched_get_state(void);

void sched_start(void);

extern unsigned int g_sched_lock;

#define SCHED_LOCK() \
	do { \
		KASSERT(g_sched_lock < SCHED_LOCK_NEST_MAX); \
		g_sched_lock++; \
	} while(0)

#define SCHED_UNLOCK() \
	do { \
		KASSERT(g_sched_lock > 0); \
	    g_sched_lock--; \
	} while (0)

#define IS_SCHED_LOCKED() (g_sched_lock>0?1:0)

#endif //__SCHED_H

