#ifndef __THREAD_H
#define __THREAD_H

#include <port.h>
#include <list.h>

#ifndef CONFIG_MAX_THREAD_NAME_LEN
#define MAX_THREAD_NAME_LEN (32)
#else
#define MAX_THREAD_NAME_LEN CONFIG_MAX_THREAD_NAME_LEN
#endif

#ifndef CONFIG_MAX_THREAD_CNT
#define MAX_THREAD_CNT (32)
#else
#define MAX_THREAD_CNT CONFIG_MAX_THREAD_CNT
#endif

#ifndef CONFIG_MIN_THREAD_CNT
#define MIN_THREAD_CNT (3)
#else
#define MIN_THREAD_CNT CONFIG_MIN_THREAD_CNT
#endif

#define MAX_THREAD_ID (MAX_THREAD_CNT - 1)

#define IDLE_THREAD_ID (0)

#define LOWEST_THREAD_PRIORITY (MAX_THREAD_CNT - 1)
#define HIGHEST_THREAD_PRIORITY (0)


#ifndef CONFIG_THREAD_DEFAULT_TIME_SLICE
#define THREAD_DEFAULT_TIME_SLICE        (5)
#else
#define THREAD_DEFALUT_TIME_SLICE CONFIG_THREAD_DEFAULT_TIME_SLICE
#endif

typedef int (*thread_start_t)(void *arg);

typedef int (*thread_main_t)(void *arg);

typedef enum {
    THREAD_SUSPENDED = 0,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_PENDING,
    THREAD_SLEEPING,
    THREAD_DEAD,
} thread_state_e;

typedef struct thread {
    struct list_node node;

    addr_t sp;
    // assume stack grows downside
    void *sp_alloc_addr;
    unsigned int stack_size;

    // priority of the thread
    unsigned int priority;

    // thread sched policy
    unsigned int sched_policy;

    // time slice assigned to the thread
    tick_t time_slice;
    // remain time for the thread
    tick_t time_remain;

    // thread entry point
    thread_start_t start_entry;
    void *start_arg;

    // thread main point
    thread_main_t main_entry;
    void *main_arg;

#ifdef THREAD_STAT
    unsigned long total_runtime;
    unsigned long switch_count;
#endif

    // thread id
    int thread_id;

    thread_state_e state;

    // misc flags
    unsigned int flags;

    // point to the list head where the thread node is pennding
    list_head_t *pending_list;

    list_head_t wait_thread_list;

    unsigned int exit_code;

    char name[MAX_THREAD_NAME_LEN];
} thread_t;



void thread_init(void);

void thread_sched_start(void);

int thread_create(const char* name, unsigned int priority, thread_main_t main_entry, void *arg, size_t stack_size, tick_t time_slice, unsigned int flags);

int thread_assume(int thid);

int thread_detach(int thid);

void thread_yield();

void thread_sched(void);

thread_t *get_cur_thread(void);

void thread_become_ready(thread_t *thread);

bool thread_can_be_preempted(void);

void set_idle_thread_id(int id);

#endif
