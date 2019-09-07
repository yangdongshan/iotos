#ifndef SEM_H
#define SEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>
#include <tick.h>

#define SEM_MAGIC   (0xFFFFFF02)

#define     SEM_OK             (0)
#define     SEM_TIMEOUT        (1)
#define     SEM_AGAIN          (2)

#define     SEM_CNT_THRESHOLD  (0xFFFF)

typedef struct sem {
    uint32_t magic;
    int32_t cnt;
    list_head_t wait_list;
} sem_t;



int sem_init(sem_t *sem, int val);

int sem_wait(sem_t *sem);

int sem_timedwait(sem_t *sem, tick_t ticks);


int sem_trywait(sem_t *sem);

int sem_post(sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif // SEM_H

