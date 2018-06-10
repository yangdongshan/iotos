#ifndef __SEMPHORE_H
#define __SEMPHORE_H

#include <list.h>

#define SEM_OK         (0)
#define ERR_SEM_PTR    (1)
#define ERR_SEM_AGAIN  (2)

typedef struct sem {
    int cnt;
    list_head_t wait_list;
} sem_t;


int sem_init(sem_t *sem, int val);

int sem_wait(sem_t *sem);

int sem_timedwait(sem_t *sem, int ms);

int sem_trywait(sem_t *sem);

int sem_post(sem_t *sem);

#endif // __SEMPHORE_H

