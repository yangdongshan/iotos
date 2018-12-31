#include <typedef.h>
#include <irq.h>
#include <task.h>
#include <sem.h>
#include <kdebug.h>
#include <workqueue.h>
#include <time.h>
#include <timer.h>
#include <string.h>

/** work queue config
 */
#ifdef CONFIG_WORKQUEU_TASK_STACK_SIZE
#define WQ_TASK_STACK_SIZE CONFIG_WORKQUEU_TASK_STACK_SIZE
#else
#define WQ_TASK_STACK_SIZE 1024
#endif

#ifdef CONFIG_WORKQUEUE_TASK_PRIORITY
#define WQ_TASK_PRIORITY
#else
#define WQ_TASK_PRIORITY 20
#endif

#define WQ_TAKS_NAME "workqueue"

#ifdef CONFIG_WORKQUEUE_DELAY
#define WORKQUEUE_DELAY CONFIG_WORKQUEUE_DELAY
#else
#define WORKQUEUE_DELAY 100
#endif

static workq_t workqueue;

static task_t wq_task;
static unsigned char wq_task_stack[WQ_TASK_STACK_SIZE];

int workqueue_init_worker(worker_t *worker, work_t do_work, void *arg, tick_t delay_ticks)
{
    if (worker == NULL)
        return WQ_INV_ARG;

    if (do_work == NULL)
        return WQ_INV_ARG;

    worker->do_work = do_work;
    worker->arg = arg;
    worker->delay = delay_ticks;

    return WQ_OK;
}

int workqueue_queue_worker(worker_t *worker)
{
    irqstate_t state;
    tick_t cur_tick;
    worker_t *iter;
    int inserted = 0;

    if (worker == NULL)
        return WQ_INV_ARG;

    state = enter_critical_section();

    cur_tick = get_sys_tick();
    worker->queue_tick = cur_tick;
    worker->timeout = cur_tick + worker->delay;

    if (list_is_empty(&workqueue.worker_list)) {
        list_add_head(&workqueue.worker_list, &worker->node);
        task_wakeup(&wq_task);
        goto out;
    } else {
        iter = list_first_entry(&workqueue.worker_list, worker_t, node);
        if (iter->timeout > worker->timeout) {
            list_add_head(&workqueue.worker_list, &worker->node);
            task_wakeup(&wq_task);
            goto out;
        }

        list_foreach_entry(&workqueue.worker_list, iter, worker_t, node) {
            if (iter->timeout > worker->timeout) {
                list_add_after(&iter->node, &worker->node);
                inserted = 1;
                break;
            }
        }

        if (inserted == 0) {
            list_add_tail(&workqueue.worker_list, &worker->node);
        }
    }

out:
    leave_critical_section(state);
    return WQ_OK;
}

int workqueue_cancel_worker(worker_t *worker)
{
    irqstate_t state;
    worker_t *iter;
    int ret = WQ_NOT_FOUND_WORKER;

    if (worker == NULL)
        return WQ_INV_ARG;

    state = enter_critical_section();

    list_foreach_entry(&workqueue.worker_list, iter, worker_t, node) {
        if (iter == worker) {
            list_delete(&iter->node);
            ret = WQ_OK;
            break;
        }
    }

    leave_critical_section(state);

    return ret;
}

int wq_process(void *arg)
{
    tick_t delay_tick;
    tick_t cur_tick;
    worker_t *iter_worker;
    irqstate_t state;
    workq_t *wq = &workqueue;

    arg = arg;

    while (1) {
        delay_tick = wq->delay;
        state = enter_critical_section();
        if (list_is_empty(&wq->worker_list)) {
            goto sleep;
        }

        for (;;) {
            cur_tick = get_sys_tick();
            iter_worker = list_first_entry(&wq->worker_list, worker_t, node);
            if (iter_worker->timeout <= cur_tick) {
                list_delete(&iter_worker->node);
                leave_critical_section(state);
                iter_worker->do_work(iter_worker->arg);
                state = enter_critical_section();
            } else {
                delay_tick = iter_worker->timeout - cur_tick;
                break;
            }
        }

sleep:
        leave_critical_section(state);

        msleep(delay_tick);
    }
}

void workqueue_init_early(void)
{
    memset(&workqueue, 0, sizeof(workq_t));

    list_head_init(&workqueue.worker_list);

    workqueue.delay = WORKQUEUE_DELAY;
}

void create_workqueue_task(void)
{
    task_create(&wq_task,
                WQ_TAKS_NAME,
                WQ_TASK_PRIORITY,
                wq_process,
                NULL,
                wq_task_stack,
                WQ_TASK_STACK_SIZE,
                5,
                TASK_AUTO_RUN);
}
