#ifndef SEM_H
#define SEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>
#include <tick.h>

#define SEM_MAGIC   (0x2de89acf)

#define     SEM_OK             (0)
#define     SEM_TIMEOUT        (1)
#define     SEM_AGAIN          (2)

typedef struct sem {
    uint32_t magic;
    int32_t cnt;
    list_head_t wait_list;
} sem_t;



int sem_init(sem_t *sem, int val);

int sem_wait(sem_t *sem);

int sem_timedwait(sem_t *sem, int ms);

int sem_trywait(sem_t *sem);

int sem_post(sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif // SEM_H

