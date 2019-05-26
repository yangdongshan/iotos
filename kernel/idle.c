#include <task.h>

/** idle task config
 */
#ifdef CONFIG_IDLE_TASK_STACK_SIZE
#define IDLE_TASK_STACK_SIZE CONFIG_IDLE_TASK_STACK_SIZE
#else
#define IDLE_TASK_STACK_SIZE  1024
#endif

static task_t idle_task;

static unsigned char idle_task_stack[IDLE_TASK_STACK_SIZE];

static int idle_main(void *arg)
{
    while (1) {
        ;;
    }

    return 0;
}

void create_idle_task(void)
{
   task_create(&idle_task, "idle",
               LOWEST_TASK_PRIORITY,
               idle_main, NULL,
               idle_task_stack,
               IDLE_TASK_STACK_SIZE,
               0, TF_AUTO_RUN);
}

