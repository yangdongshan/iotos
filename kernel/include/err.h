#ifndef __ERRNO_H
#define __ERRNO_H

typedef enum {
    NO_ERR = 0,

    ERR_INVALID_TASK_ID,
    ERR_INVALID_TASK_ENTRY,
    ERR_NO_ENOUGH_TASK_ID,

    ERR_SUSPEDN_IDLE_TASK,


    ERR_NULL_PTR,
    ERR_INVALID_ARG,
    ERR_NO_MEM,

    ERR_IN_INTERRUPT,
} errno_t;

#endif // __ERRNO_H

