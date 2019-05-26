#ifndef __MUTEX_H
#define __MUTEX_H

#include <task.h>

#define MUTEX_MAGIC (0x435fdec3)

typedef struct {
    uint32_t magic;
    uint32_t cnt;
    task_t *holder;
    struct list_node wait_list;
} mutex_t;

int mutex_init(mutex_t *mutex);

int mutex_wait(mutex_t *mutex);

int mutex_timedwait(mutex_t *mutex, tick_t ticks);

int mutex_trywait(mutex_t *mutex);

int mutex_post(mutex_t *mutex);

#endif // __MUTEX_H

