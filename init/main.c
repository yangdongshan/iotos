#include <typedef.h>
#include <kdebug.h>
#include <board.h>
#include <testcase.h>
#include <task.h>
#include <tick.h>
#include <sem.h>
#include <mutex.h>
#include <ringbuf.h>
#include <workqueue.h>
#include <platform.h>

extern void os_init(void);
extern void os_run(void);

#define SYS_INIT_STACK_SIZE 1024
task_t sys_init;
static unsigned char sys_init_stack[SYS_INIT_STACK_SIZE];

static int sys_init_run(void *arg)
{
    int ret;
    int cnt = 0;

    KINFO("board_init\r\n");
    board_init();

    systick_start();

    while(1) {
        kdebug_print("%s: sys_init loop %d\r\n", __func__, cnt);
        cnt++;

        task_sleep(10);
    }

    return ret;
}

int os_start()
{
    platform_init();

    KINFO("*** welcome to iotos *** \r\n\r\n");

    os_init();

    task_create(&sys_init,
                "sys_init",
                10,
                sys_init_run,
                NULL,
                sys_init_stack,
                SYS_INIT_STACK_SIZE,
                5,
                TF_AUTO_RUN);

    os_run();

    while (1) {
        KASSERT(0);
    }

    return 0;
}

