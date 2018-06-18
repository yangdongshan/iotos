#include <time.h>
#include <timer.h>
#include <thread.h>
#include <irq.h>
#include <err.h>
#include <kdebug.h>

volatile unsigned long ticks = 0;

void sys_time_tick(void)
{
    ticks++;
}

tick_t get_sys_tick(void)
{
    return ticks;
}


int msleep(unsigned int ms)
{
    irqstate_t state;

    thread_t *thread = get_cur_thread();

    KDBG(DEBUG, "thread %s go to sleep\r\n", thread->name);

    int ret = register_oneshot_timer(ms, thread_become_ready, thread);

    state = enter_critical_section();
    thread->state = THREAD_SLEEPING;
    thread_sched();

    leave_critical_section(state);


    return NO_ERR;
}

