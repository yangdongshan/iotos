#include <task.h>
#include <timer.h>
#include <mm.h>

#define IDLE_TASK_STACK_SIZE  1024
task_t idle_task;
static unsigned char idle_task_stack[IDLE_TASK_STACK_SIZE];


static int idle_main(void *arg)
{
    while (1) {
        ;;
    }

    return 0;
}

void os_init(void)
{
    mm_init();

    timer_init();

    task_init();

   int id = task_create(&idle_task,
                        "idle",
                        LOWEST_TASK_PRIORITY,
                        idle_main,
                        NULL,
                        idle_task_stack,
                        IDLE_TASK_STACK_SIZE,
                        5, 0);

   if (id != -1) {
        set_idle_task_id(id);
   }
}

void os_run(void)
{
    task_sched_start();
}
