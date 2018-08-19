#include <typedef.h>
#include <irq.h>
#include <task.h>
#include <sem.h>
#include <kdebug.h>
#include <workqueue.h>
#include <time.h>
#include <timer.h>
#include <string.h>

#ifdef CONFIG_WORKQUEUE_DELAY
#define WORKQUEUE_DELAY CONFIG_WORKQUEUE_DELAY
#else
#define WORKQUEUE_DELAY 100
#endif

static workq_t workqueue;

int workqueue_init_worker(worker_t *worker, work_t do_work, void *arg, int delay_ms)
{
    if (worker == NULL)
        return WQ_INV_ARG;

    if (do_work == NULL)
        return WQ_INV_ARG;

    worker->do_work = do_work;
    worker->arg = arg;
    worker->delay = MS2TICKS(delay_ms);

    return WQ_OK;
}

int workqueue_queue_worker(worker_t *worker)
{
    irqstate_t state;

    if (worker == NULL)
        return WQ_INV_ARG;

    tick_t cur_tick = get_sys_tick();

    state = enter_critical_section();

    worker->queue_tick = cur_tick;
    list_add_tail(&workqueue.worker_list, &worker->node);

    leave_critical_section(state);

    return WQ_OK;
}

int workqueue_cancel_worker(worker_t *worker)
{
    irqstate_t state;
    int ret;

    if (worker == NULL)
        return WQ_INV_ARG;

    state = enter_critical_section();

    if (list_is_empty(&workqueue.worker_list)) {
        leave_critical_section(state);
        return WQ_NOT_FOUND_WORKER;
    }

    struct list_node *next_node;
    struct list_node *cur_node;

    ret = WQ_NOT_FOUND_WORKER;
    next_node = workqueue.worker_list.next;
    while (next_node != &workqueue.worker_list) {
        cur_node = next_node;
        next_node = next_node->next;
        worker_t *iter_worker;
        iter_worker = list_entry(cur_node, worker_t, node);
        if (iter_worker == worker) {
            list_delete(&iter_worker->node);
            ret = WQ_OK;
            break;
        }
    }

    leave_critical_section(state);

    return ret;
}

int wq_process(void *arg)
{
    arg = arg;

    workq_t *wq = &workqueue;
    tick_t delay_tick;
    tick_t min_remain_tick = 0xffffffff;
    tick_t remain_tick;
    tick_t cur_tick, prev_tick;
    irqstate_t state;

    delay_tick = wq->delay;
    min_remain_tick = delay_tick;

    struct list_node *next_node;
    struct list_node *cur_node;
    worker_t *iter_worker;
    while (1) {
        state = enter_critical_section();
        if (list_is_empty(&wq->worker_list)) {
            min_remain_tick = delay_tick;
        } else {
            cur_tick = get_sys_tick();
            next_node = wq->worker_list.next;
            while (next_node != &wq->worker_list) {
                cur_node = next_node;
                next_node = next_node->next;
                prev_tick = cur_tick;
                cur_tick = get_sys_tick();
                min_remain_tick -= cur_tick - prev_tick;
                iter_worker = list_entry(cur_node, worker_t, node);
                remain_tick = cur_tick - iter_worker->queue_tick;
                if (remain_tick >= iter_worker->delay) {
                    list_delete(&iter_worker->node);
                    leave_critical_section(state);
                    iter_worker->do_work(iter_worker->arg);
                    state = enter_critical_section();
                } else {
                    if (min_remain_tick > remain_tick) {
                        min_remain_tick = remain_tick;
                    }
                }

                if (min_remain_tick > delay_tick) {
                    min_remain_tick = delay_tick;
                }
            }
        }

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

