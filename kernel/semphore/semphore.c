#include <semphore.h>
#include <typedef.h>

int sem_init(sem_t *sem, int val)
{
    int ret = SEM_OK;
    irqstate_t state;

    if (sem == NULL) {
        return -ERR_SEM_PTR;
    }

    state = enter_critical_section();

    list_head_init(&sem->wait_list);
    sem->cnt = val;

    leave_critical_senction(state);
    return ret;
}

int sem_wait(sem_t *sem)
{
    int ret = SEM_OK;
    irqstate_t state;

    if (sem == NULL) {
        return -ERR_SEM_PTR;
    }

    state = enter_critical_section();

    leave_critical_senction(state);

    return ret;
}

int sem_timedwait(sem_t *sem, int ms)
{
    int ret = SEM_OK;
    irqstate_t state;

    if (sem == NULL) {
        return -ERR_SEM_PTR;
    }

    state = enter_critical_section();

    leave_critical_senction(state);

    return ret;
}

int sem_trywait(sem_t *sem)
{
    int ret = SEM_OK;
    irqstate_t state;

    if (sem == NULL) {
        return -ERR_SEM_PTR;
    }

    state = enter_critical_section();

    if (sem->cnt <= 0) {
        ret = -ERR_SEM_AGAIN;
    } else {
        sem->cnt--;
    }

    leave_critical_senction(state);

    return ret;
}

int sem_post(sem_t *sem)
{
    irqstate_t state;
    int ret = SEM_OK;

    if (sem == NULL) {
        return -ERR_SEM_PTR;
    }

    state = enter_critical_section();

    if (sem->cnt >= 0) {
        ret = SEM_OK;
    } else {
        // FIXME: wakeup thread waiting for the sem
    }

    sem->cnt++;

    leave_critical_senction(state);

    return ret;

}


