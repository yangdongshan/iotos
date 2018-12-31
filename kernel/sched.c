#include <irq.h>
#include <task.h>
#include <kdebug.h>
#include <sched.h>

static unsigned int os_state = OS_RESET;

void os_set_state(unsigned int state)
{
    irqstate_t irq_state;

    KASSERT(state <= OS_STOPPED);

    irq_state = enter_critical_section();

    os_state = state;

    leave_critical_section(irq_state);

    return;
}

unsigned int os_get_state(void)
{
    return os_state;
}

void os_start_sched(void)
{
    task_t *task;
    irqstate_t state;

    state = enter_critical_section();

    task = get_new_task();
    task->state = TASK_RUNNING;
    g_new_task = task;

    os_set_state(OS_RUNNING);
    arch_start_first_task();
    leave_critical_section(state);


    KERR("shouldn't return here\r\n");
    KASSERT(0);
}

