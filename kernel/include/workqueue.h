#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#ifdef __cpulsplus
extern "C" {
#endif

#include <list.h>
#include <task.h>

#define WQ_OK                   0
#define WQ_INV_ARG              1
#define WQ_NOT_FOUND_WORKER     3

typedef struct {
    // work queue list head
    list_head_t worker_list;
    // poll delay ticks
    tick_t delay;
} workq_t;

typedef void (*work_t)(void *arg);

typedef struct {
    struct list_node node;
    // the time when the work is queued
    tick_t queue_tick;
    // do the work after ticks
    tick_t delay;
    // the time when the work is timeout
    tick_t timeout;
    // work function
    work_t do_work;
    // work arg
    void *arg;
} worker_t;

int workqueue_init_worker(worker_t *worker, work_t do_work, void *arg, tick_t delay_ticks);

int workqueue_queue_worker(worker_t *worker);

int workqueue_cancel_worker(worker_t *worker);

int wq_process(void *arg);

void create_workqueue_task(void);

void workqueue_init_early(void);

#ifdef __cpulsplus
}
#endif

#endif // WORKQUEUE_H

