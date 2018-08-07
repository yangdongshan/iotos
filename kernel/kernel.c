#include <thread.h>
#include <timer.h>
#include <mm.h>

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

    thread_init();

   int id = thread_create("idle",
                          LOWEST_THREAD_PRIORITY,
                          idle_main,
                          NULL, 1024, 5, 0);

   if (id != -1) {
        set_idle_thread_id(id);
   }
}

void os_run(void)
{
    thread_sched_start();
}
