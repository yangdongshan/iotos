#ifndef __ERRNO_H
#define __ERRNO_H

typedef enum {
    ERR_OK = 0,
    ERR_INV_ARG,
    ERR_NO_MEM,

	ERR_TASK = 0x1000,
	/* trying to create more that one idle task */
	ERR_IDLE_EXITS,
	/* trying to suspend idle task */
	ERR_SUSPEND_IDLE,
	/* task pending timeout */
	ERR_WAIT_TIMEOUT,
	/* task pending state is interrupted */
	ERR_INT,

	ERR_SCHED = 0x2000,
	ERR_SCHED_LOCKED,

	ERR_INTRPT = 0x3000,
    ERR_IN_INTRPT,
    ERR_INTRPT_NESTED,

	ERR_TICK = 0x4000,
	ERR_TICK_OVFLOW,

	ERR_MUTEX = 0x5000,
	ERR_MUTEX_MAGIC,
	ERR_MUTEX_TIMEOUT,
	ERR_MUTEX_BUSY,
	ERR_MUTEX_NESTED,
	ERR_MUTEX_OVERFLOW,
	ERR_MUTEX_HOLDER,
	ERR_MUTEX_PEND_TIMEOUT_RESUME,

	ERR_SEM = 0x6000,
	ERR_SEM_MAGIC,
	ERR_SEM_TIMEOUT,
	ERR_SEM_BUSY,
	ERR_SEM_OVERFLOW,
	ERR_SEM_PEND_TIMEOUT_RESUME,
} errno_t;

#endif // __ERRNO_H

