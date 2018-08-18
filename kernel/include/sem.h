#ifndef SEM_H
#define SEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>
#include <timer.h>


#define     SEM_OK             (0)
#define     SEM_TIMEOUT        (1)
#define     SEM_AGAIN          (2)
#define     SEM_HOLDER_ERR     (3)

typedef struct sem {
    int cnt;
    list_head_t wait_list;
    int wait_cnt;
    list_head_t holder_list;
    int holder_cnt;
    timer_t *timer;
} sem_t;


void sem_init_early(void);

int sem_init(sem_t *sem, int val);

int sem_wait(sem_t *sem);

int sem_timedwait(sem_t *sem, int ms);

int sem_trywait(sem_t *sem);

int sem_post(sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif // SEM_H

