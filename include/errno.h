#ifndef __ERRNO_H
#define __ERRNO_H

typedef enum {
    NO_ERR = 0,

    ERR_INVALID_THREAD_ID,
    ERR_NO_THREAD_ID_AVAILABLE,
    ERR_SUSPEDN_IDLE_THREAD,

    ERR_NO_MEM,
} errno_t;

#endif // __ERRNO_H

