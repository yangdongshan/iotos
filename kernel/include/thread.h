#ifndef __THREAD_H
#define __THREAD_H

#include <types.h>
#include <list.h>

typedef int (*thread_entry_t)(void);

typedef int (*thread_main_t)(int argc, char**argv);


typedef enum {
    THREAD_SUSPENDED = 0,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_PENDING,
    THREAD_SLEEPING,
    THREAD_DEAD,
} thread_state_e;

typedef struct {
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
    time_t time_slice;
    // remain time for the thread
    time_t time_remain;

    // thread entry point
    thread_entry_t thread_entry;

    // thread main point
    thread_main_t thread_main;

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
    list_head *pending_list;

    list_head wait_thread_list;

    unsigned int exit_code;

    unsigned char name[MAX_THREAD_NAME_LEN];
} thread_t;


int thread_create(const char* name, thread_main_t entry, addr_t *stack, size_t stack_size, void *arg);

int thread_assume(int thid);

int thread_detach(int thid);

int thread_yield();


#endif
