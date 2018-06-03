#ifndef __THREAD_H
#define __THREAD_H

#include <type_def.h>
#include <context.h>
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

typedef int (*thread_start_t)(void);

typedef int (*thread_main_t)(int argc, char**argv);


typedef enum {
    THREAD_SUSPENDED,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_PENDING,
    THREAD_SLEEPING,
    THREAD_DEAD,
} thread_state_e;

typedef struct thread {
    struct list_node node;

    addr_t *sp;
    // assume stack grows downside
    addr_t *sp_bottom;
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
    unsigned int thread_id;

    thread_state_e state;

    // misc flags
    unsigned int flags;

    // point to the list head where the thread node is pennding
    list_head_t *pending_list;

    list_head_t wait_thread_list;

    unsigned int exit_code;

    unsigned char name[MAX_THREAD_NAME_LEN];
} thread_t;

int thread_create(const char* name, unsigned int priority, thread_main_t main_entry, void *arg, size_t stack_size, tick_t time_slice, unsigned int flags);

int thread_assume(int thid);

int thread_detach(int thid);

void thread_yield();


#endif
