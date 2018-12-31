#ifndef __TASK_H
#define __TASK_H

#include <port.h>
#include <list.h>
#include <timer.h>

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


#define TASK_AUTO_RUN   0x01ul
#define TASK_IS_AUTO_RUN(f) (f & 0x01)

#define TF_RESCHED      (1ul << 31)
#define TASK_RESCHED_SET(t) (t->flags |= TF_RESCHED)
#define TASK_RESCHED_CLR(t) (t->flags &= (~TF_RESCHED))
#define TASK_NEED_RESCHED(t) (t->flags & TF_RESCHED)

typedef int (*task_start_t)(void *arg);

typedef int (*task_main_t)(void *arg);

#define TASK_SUSPENDED      (0x00UL)
#define TASK_READY          (0x01UL)
#define TASK_RUNNING        (0x02UL)
#define TASK_PENDING        (0x03UL)
#define TASK_SLEEPING       (0x04UL)
#define TASK_DEAD           (0x05UL)


#define    PEND_NONE        (0x00UL)
#define    PEND_TIMEOUT     (0x01UL)
#define    PEND_WAKEUP      (0x02UL)

typedef struct task {
    addr_t stack;

    struct list_node node;

    // assume stack grows downside
    void *sp_alloc_addr;
    unsigned int stack_size;

    // priority of the task
    unsigned int priority;

    // original priority
    unsigned int origin_priority;

    // task sched policy
    unsigned int sched_policy;

    // time slice assigned to the task
    tick_t time_slice;
    // remain time for the task
    tick_t time_remain;

    // task entry point
    task_start_t start_entry;
    void *start_arg;

    // task main point
    task_main_t main_entry;
    void *main_arg;

#ifdef CONFIG_TASK_STAT
    unsigned long total_runtime;
    unsigned long switch_count;
#endif

    unsigned int state;

    // misc flags
    unsigned int flags;

    timer_t wait_timer;
    // point to the list head where the task node is pennding
    list_head_t *pending_list;

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

int task_create(task_t *task,
                const char* name,
                unsigned int priority,
                task_main_t main_entry,
                void *arg,
                void *stack,
                size_t stack_size,
                tick_t time_slice,
                unsigned int flags);

int task_resume(task_t *task);

int task_detach(int thid);

void task_yield();

void task_switch(void);

task_t *get_cur_task(void);

int task_wakeup(task_t *task);

void task_become_ready_head(task_t *task);

void task_become_ready_tail(task_t *task);

bool task_can_be_preempted(void);

void task_set_priority(task_t *task, int priority);

void task_restore_priority(task_t *task);

void task_list_add_priority(struct list_node *head, task_t *task);

#endif
