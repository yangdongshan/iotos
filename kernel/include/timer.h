#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>

typedef int (*timeout_cb)(void *arg);

typedef struct timer {
    struct list_node node;
    unsigned int timeout;
    unsigned int cycle;
    timeout_cb timeout_handle;
    void *arg;
    int flag;
    char *name;
} timer_t;

void timer_init_early(void);

void timer_tick(void);

timer_t* register_oneshot_timer(timer_t *timer,
                    char *name,
                    unsigned int delay,
                    timeout_cb handle,
                    void *arg);

timer_t* register_periodical_timer(timer_t *timer,
                    char *name,
                    unsigned int delay,
                    timeout_cb handle,
                    void *arg);

int cancel_timer(timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif // __TIMER_H
