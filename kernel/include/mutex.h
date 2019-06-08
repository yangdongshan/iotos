#ifndef __MUTEX_H
#define __MUTEX_H

#include <task.h>

#define MUTEX_MAGIC (0xFFFFFF01)

typedef struct {
    uint32_t magic;
    uint32_t cnt;
    task_t *holder;
    struct list_node wait_list;
} mutex_t;

int mutex_init(mutex_t *mutex);

int mutex_lock(mutex_t *mutex);

int mutex_timedlock(mutex_t *mutex, tick_t ticks);

int mutex_trylock(mutex_t *mutex);

int mutex_unlock(mutex_t *mutex);

#endif // __MUTEX_H

