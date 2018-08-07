#include <time.h>
#include <timer.h>
#include <thread.h>
#include <irq.h>
#include <err.h>
#include <kdebug.h>

unsigned long gticks = 0;



int msleep(unsigned int ms)
{
    irqstate_t state;

    state = enter_critical_section();
    thread_t *thread = get_cur_thread();

    KDBG(DEBUG, "thread %s go to sleep\r\n", thread->name);

    register_oneshot_timer(thread->name, ms, thread_become_ready, thread);

    thread->state = THREAD_SLEEPING;
    thread_sched();

    leave_critical_section(state);


    return NO_ERR;
}

