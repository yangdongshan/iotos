#include <stdio.h>
#include <typedef.h>
#include <kdebug.h>
#include <arch.h>
#include <soc.h>
#include <board.h>
#include <kernel.h>
#include <stm32f4xx_conf.h>
#include <testcase.h>

static int sys_init(void *arg);


int os_start()
{
    arch_init();

    KDBG(INFO, "*** welcome to iotos *** \r\n\r\n");

    KDBG(INFO, "init board\r\n");
    board_init();

    mm_init();

    KDBG(INFO, "os_init\r\n");

    thread_early_init();

    thread_create("sys_init", 10, sys_init, NULL, 1024, 0, 0);

    thread_sched_start();

    while (1) {
        KASSERT(0);
    }

    return 0;
}


static int sys_init(void *arg)
{
    int ret;
    int cnt = 0;

    while(1) {
        kdebug_print("%s: sys_init loop %d\r\n", __func__, cnt++);
        thread_yield();
    }

    return ret;
}
