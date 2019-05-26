#include <irq.h>
#include <task.h>
#include <kdebug.h>
#include <sched.h>

unsigned int g_sched_lock = 0;

static unsigned int sched_state = SCHED_RESET;

void sched_set_state(unsigned int state)
{
    irqstate_t irq_state;

    KASSERT(state <= SCHED_STOPPED);

    irq_state = enter_critical_section();

    sched_state = state;

    leave_critical_section(irq_state);

    return;
}

unsigned int sched_get_state(void)
{
    return sched_state;
}

void sched_start(void)
{
    task_t *task;
    irqstate_t state;

    state = enter_critical_section();

    task = get_prefer_task();
    task->state = TS_RUNNING;
    g_new_task = task;

    sched_set_state(SCHED_RUNNING);
    arch_start_first_task();
    leave_critical_section(state);


    KERR("shouldn't return here\r\n");
    KASSERT(0);
}

