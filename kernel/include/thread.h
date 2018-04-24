#ifndef __THREAD_H
#define __THREAD_H

#include <types.h>
#include <list.h>

typedef int (*thread_entry_t)(void);

typedef int (*thread_main_t)(int argc, char**argv);


typedef enum {
    THREAD_READY = 0,
    THREAD_PENDING,
    THREAD_RUNNING,
    THREAD_SLEEPING,
    THREAD_DEAD,
} thread_state_e;

typedef struct {
    list_head node;

    addr_t *stack;
    addr_t *stack_bottom;
    unsigned int stack_size;

    // priority of the thread
    int priority;

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

    unsigned int thid;

    thread_state_e state;

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
