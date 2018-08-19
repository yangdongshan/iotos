#include <task.h>
#include <timer.h>
#include <mm.h>
#include <sem.h>
#include <workqueue.h>

/** idle task config
 */
#ifdef CONFIG_IDLE_TASK_STACK_SIZE
#define IDLE_TASK_STACK_SIZE CONFIG_IDLE_TASK_STACK_SIZE
#else
#define IDLE_TASK_STACK_SIZE  1024
#endif

/** work queue config
 */

#ifdef CONFIG_WORKQUEU_TASK_STACK_SIZE
#define WQ_TASK_STACK_SIZE CONFIG_WORKQUEU_TASK_STACK_SIZE
#else
#define WQ_TASK_STACK_SIZE 1024
#endif

#ifdef CONFIG_WORKQUEUE_TASK_PRIORITY
#define WQ_TASK_PRIORITY
#else
#define WQ_TASK_PRIORITY 20
#endif

#define WQ_TAKS_NAME "workqueue"


task_t wq_task;
static unsigned char wq_task_stack[WQ_TASK_STACK_SIZE];

task_t idle_task;
static unsigned char idle_task_stack[IDLE_TASK_STACK_SIZE];

static int idle_main(void *arg)
{
    while (1) {
        ;;
    }

    return 0;
}

static void create_idle_task(void)
{
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

static void create_workqueue_task(void)
{
    task_create(&wq_task,
                WQ_TAKS_NAME,
                WQ_TASK_PRIORITY,
                wq_process,
                NULL,
                wq_task_stack,
                WQ_TASK_STACK_SIZE,
                5,
                0);
}

void os_init(void)
{
    mm_init_early();

    timer_init_early();

    task_init_early();

    sem_init_early();

    workqueue_init_early();

    create_idle_task();

    create_workqueue_task();
}

void os_run(void)
{
    task_sched_start();
}
