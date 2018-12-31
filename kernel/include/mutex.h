#ifndef __MUTEX_H
#define __MUTEX_H

#include <task.h>

#define MUTEX_MAGIC (0x435fdec3)

#define MUTEX_OK          (0x00UL)
#define MUTEX_ETIMEOUT    (0x01UL)
#define MUTEX_EAGAIN      (0x02UL)
#define MUTEX_ENESTED     (0x03UL)
#define MUTEX_EHOLDER     (0x04UL)
#define MUTEX_ECNT        (0x05UL)
#define MUTEX_EMAGIC      (0x06UL)


typedef struct {
    int magic;
    int cnt;
    task_t *holder;
    struct list_node wait_list;
} mutex_t;

int mutex_init(mutex_t *mutex);

int mutex_wait(mutex_t *mutex);

int mutex_timedwait(mutex_t *mutex, unsigned long ticks);

int mutex_trywait(mutex_t *mutex);

int mutex_post(mutex_t *mutex);

#endif // __MUTEX_H

