#ifndef __TASK_H
#define __TASK_H

#include <port.h>
#include <list.h>
#include <tick.h>

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


#define TF_IDLE_TASK  0x01u
#define TF_AUTO_RUN   0x02u


#define TF_RESCHED      (1ul << 31)
#define TASK_RESCHED_SET(t) (t->flags |= TF_RESCHED)
#define TASK_RESCHED_CLR(t) (t->flags &= (~TF_RESCHED))
#define TASK_NEED_RESCHED(t) (t->flags & TF_RESCHED)

typedef int (*task_entry_t)(void *arg);

#define TS_INIT           				(0x00UL)	

#define TS_READY          				(0x01UL)

#define TS_RUNNING        				(0x02UL)

#define TS_SUSPENDED      				(0x03UL)

#define TS_PEND_MUTEX     			    (0x04UL)
#define TS_PEND_MUTEX_SUSPEND			(0x05UL)
#define TS_PEND_MUTEX_TICK				(0x06UL)
#define TS_PEND_MUTEX_TICK_SUSPEND		(0x07UL)

#define TS_PEND_SEM						(0x08UL)
#define TS_PEND_SEM_SUSPEND				(0x09UL)
#define TS_PEND_SEM_TICK				(0x0AUL)
#define TS_PEND_SEM_TICK_SUSPEND		(0x0BUL)

// TODO: pend queue, event

#define TS_PEND_SLEEP          			(0x14UL)
#define TS_PEND_SLEEP_SUSPEND  			(0x15UL)

#define TASK_DEAD           			(0x16UL)


#define PEND_OK          (0x00UL)
#define PEND_TIMEOUT     (0x01UL)
#define PEND_WAKEUP      (0x02UL)
#define PEND_INT         (0x03UL)
#define PEND_CANCEL      (0x04UL)

typedef struct task {
    addr_t stack;

    struct list_node node;

    // assume stack grows downside
    addr_t sp_alloc_addr;
    unsigned long stack_size;

    // priority of the task
    int prio;

    // original priority
    int origin_prio;

    // task sched policy
    unsigned int sched_policy;

    // time slice assigned to the task
    unsigned int time_slice;
    // remain time for the task
    unsigned int time_remain;

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

    unsigned long state;

    // misc flags
    unsigned long flags;

    // point to the list head where the task node is pending on
    list_head_t *pend_list;

	// when the task is blocked
	tick_t pend_time;
	// how many ticks the task will block
	tick_t pend_timeout;
	
    int pend_ret_code;

    list_head_t wait_task_list;

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

int task_switch(void);

task_t *get_cur_task(void);

int task_wakeup(task_t *task);

void task_addto_ready_list_tail(task_t *task);

void task_become_ready(task_t *task);

int task_need_resched(void);

void task_set_priority(task_t *task, int priority);

void task_restore_priority(task_t *task);

void task_list_add_priority(struct list_node *head, task_t *task);

#endif
