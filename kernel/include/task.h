#ifndef __TASK_H
#define __TASK_H

#include <port.h>
#include <list.h>

#ifndef CONFIG_MAX_TASK_NAME_LEN
#define MAX_TASK_NAME_LEN (32)
#else
#define MAX_TASK_NAME_LEN CONFIG_MAX_TASK_NAME_LEN
#endif

#ifndef CONFIG_MAX_TASK_CNT
#define MAX_TASK_CNT (32)
#else
#define MAX_TASK_CNT CONFIG_MAX_TASK_CNT
#endif

#ifndef CONFIG_MIN_TASK_CNT
#define MIN_TASK_CNT (3)
#else
#define MIN_TASK_CNT CONFIG_MIN_TASK_CNT
#endif

#define MAX_TASK_ID (MAX_TASK_CNT - 1)

#define IDLE_TASK_ID (0)

#define LOWEST_TASK_PRIORITY (MAX_TASK_CNT - 1)
#define HIGHEST_TASK_PRIORITY (0)


#ifndef CONFIG_TASK_DEFAULT_TIME_SLICE
#define TASK_DEFAULT_TIME_SLICE        (5)
#else
#define TASK_DEFALUT_TIME_SLICE CONFIG_TASK_DEFAULT_TIME_SLICE
#endif


#define TF_IDLE_TASK  0x01u /**< idle task */
#define TF_AUTO_RUN   0x02u /**< task auto run */
#define TF_TASK_MM    0x04u /**< task struct from heap */
#define TF_STACK_MM   0X08u /**< task stack from heap */


#define TF_RESCHED      (1ul << 31)
#define TASK_RESCHED_SET(t) (t->flags |= TF_RESCHED)
#define TASK_RESCHED_CLR(t) (t->flags &= (~TF_RESCHED))
#define TASK_NEED_RESCHED(t) (t->flags & TF_RESCHED)

typedef int (*task_entry_t)(void *arg);

#define TS_INIT           				(0x00UL)

#define TS_READY          				(0x01UL)

#define TS_RUNNING        				(0x02UL)

#define TS_SUSPEND       				(0x03UL)

/* task is waiting for mutex */
#define TS_PEND_MUTEX     			    (0x04UL)

/* task is suspend while waiting for mutex */
#define TS_PEND_MUTEX_SUSPEND			(0x05UL)

/* task is suspened by waiting for mutex timeout */
#define TS_PEND_MUTEX_TIMEOUT_SUSPEND   (0x06UL)

#define TS_PEND_SEM						(0x07UL)
#define TS_PEND_SEM_SUSPEND				(0x08UL)
#define TS_PEND_SEM_TIMEOUT_SUSPEND     (0x09UL)


// TODO: pend queue, event

#define TS_PEND_SLEEP          			(0x0AUL)
#define TS_PEND_SLEEP_SUSPEND  			(0x0BUL)

#define TS_TASK_DEAD           			(0x0CUL)

#define PEND_OK          (0x00UL)
#define PEND_TIMEOUT     (0x01UL)
#define PEND_WAKEUP      (0x02UL)
#define PEND_RESUME      (0x03UL)
#define PEND_INT         (0x04UL)
#define PEND_CANCEL      (0x05UL)


typedef struct task {
    void *stack;

    struct list_node node;

    // assume stack grows downside
    void *sp_alloc_addr;
    size_t stack_size;

    // priority of the task
    uint8_t prio;

    // original priority
    uint8_t origin_prio;

    // time slice assigned to the task
    uint32_t time_slice;
    // remain time for the task
    uint32_t time_remain;

    // task entry point
    task_entry_t start_entry;
    void *start_arg;

    // task main point
    task_entry_t main_entry;
    void *main_arg;

#ifdef CONFIG_TASK_STAT
    unsigned long total_runtime;
    unsigned long switch_count;
#endif

    uint16_t state;

    // misc flags
    uint16_t flags;

	// the node to be linked in mutex/sem/queue waiter list
	struct list_node pend_node;

    // point to the list head where the task node is pending on
    list_head_t *pend_list;

	// how many ticks the task will block
	tick_t pend_timeout;

    int pend_ret_code;

    unsigned int exit_code;

    const char *name;
} task_t;



extern task_t *g_cur_task;
extern task_t *g_new_task;

void task_init_early(void);


static inline void task_set_state(task_t *task, unsigned int state)
{
    task->state = state;
}

static inline unsigned int task_get_state(task_t *task)
{
    return task->state;
}

int task_sleep(tick_t ticks);

int task_create(task_t *task,
                const char* name,
                int prio,
                task_entry_t entry,
                void *arg,
                void *stack,
                size_t stack_size,
                unsigned int time_slice,
                unsigned int flags);


int task_resume(task_t *task);

int task_detach(int thid);

void task_yield();

int task_exit(int exit_code);

int task_switch(void);

task_t *get_cur_task(void);

int task_wakeup(task_t *task);

void task_addto_ready_list_tail(task_t *task);

void task_ready_list_remove(task_t *task);

void task_addto_suspend_list(task_t *task);

void task_become_ready(task_t *task);

int task_need_resched(void);

void task_set_prio(task_t *task, int priority);

void task_restore_prio(task_t *task);

void task_list_add_prio(struct list_node *head, struct list_node *node, int prio);

int task_pend_ret_code_convert(int pend_ret_code);

#endif
