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


#define AUTO_RUN   0x01ul
#define TASK_IS_AUTO_RUN(f) (f & 0x01)


typedef int (*task_start_t)(void *arg);

typedef int (*task_main_t)(void *arg);

typedef enum {
    TASK_SUSPENDED = 0,
    TASK_READY,
    TASK_RUNNING,
    TASK_PENDING,
    TASK_SLEEPING,
    TASK_DEAD,
} task_state_e;

typedef enum {
    PEND_NONE = 0,
    PEND_TIMEOUT,
    PEND_WAKEUP,
} pend_ret_code_t;

typedef struct task {
    struct list_node node;

    addr_t sp;
    // assume stack grows downside
    void *sp_alloc_addr;
    unsigned int stack_size;

    // priority of the task
    unsigned int priority;

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

    // task id
    int task_id;

    task_state_e state;

    // misc flags
    unsigned int flags;

    timer_t wait_timer;
    // point to the list head where the task node is pennding
    list_head_t *pending_list;

    int pend_ret_code;

    list_head_t wait_task_list;

    unsigned int exit_code;

    char *name;
} task_t;



void task_init_early(void);

void task_sched_start(void);

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

void task_sched(void);

task_t *get_cur_task(void);

void task_become_ready_head(task_t *task);

void task_become_ready_tail(task_t *task);

bool task_can_be_preempted(void);

void set_idle_task_id(int id);

#endif
