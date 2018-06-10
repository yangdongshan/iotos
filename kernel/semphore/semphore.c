#include <semphore.h>
#include <kernel.h>
#include <typedef.h>

int sem_init(sem_t *sem, int val)
{
    if (sem == NULL) {
        return -ERR_SEM_PTR;
    }

    list_head_init(&sem->wait_list);
    sem->cnt = val;

    return SEM_OK;
}

int sem_wait(sem_t *sem)
{
    irqstate_t state;


    return SEM_OK;
}

int sem_timedwait(sem_t *sem, int ms)
{
    return SEM_OK;
}

int sem_trywait(sem_t *sem)
{
    return SEM_OK;

}

int sem_post(sem_t *sem)
{
    return SEM_OK;

}


