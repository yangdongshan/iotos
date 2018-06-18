#ifndef __ERRNO_H
#define __ERRNO_H

typedef enum {
    NO_ERR = 0,

    ERR_INVALID_THREAD_ID,
    ERR_INVALID_THREAD_ENTRY,
    ERR_NO_THREAD_ID_AVAILABLE,

    ERR_SUSPEDN_IDLE_THREAD,


    ERR_NULL_PTR,
    ERR_INVALID_ARG,
    ERR_NO_MEM,

    ERR_IN_INTERRUPT,
} errno_t;

#endif // __ERRNO_H

