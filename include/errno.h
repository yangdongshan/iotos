#ifndef __ERRNO_H
#define __ERRNO_H

#define NO_ERR  (0)

typedef enum {
    NO_ERR = 0,

    ERR_NO_FREE_THREAD_NODE,

    ERR_INVALID_THREAD_ID,

    ERR_SUSPEDN_IDLE_THREAD,

} errno_t;

#endif // __ERRNO_H

