#include <stdio.h>
#include <typedef.h>
#include <kdebug.h>
#include <board.h>
#include <kernel.h>
#include <stm32f4xx_conf.h>
#include <testcase.h>
#include <timer.h>

int my_timer1(void *arg)
{
    static int cnt = 1;

    kdebug_print("timer1 cnt %d\r\n", cnt);
    cnt++;

    return 0;
}

int my_timer2(void *arg)
{
    static int cnt = 1;

    kdebug_print("timer2 cnt %d\r\n", cnt);
    cnt++;

    return 0;
}
static int sys_init(void *arg)
{
    int ret;
    int cnt = 0;

    KDBG(INFO, "board_init\r\n");
    board_init();

    arch_systick_start();

    register_periodical_timer(1000, my_timer1, NULL);
    while(1) {
        kdebug_print("%s: sys_init loop %d\r\n", __func__, cnt++);
        msleep(1000);
        //thread_yield();
    }

    return ret;
}

int os_start()
{
    arch_init();

    KDBG(INFO, "*** welcome to iotos *** \r\n\r\n");

    KDBG(INFO, "os_init\r\n");

    os_init();

    thread_create("sys_init", 10, sys_init, NULL, 1024, 5, 0);

    os_run();

    while (1) {
        KASSERT(0);
    }

    return 0;
}



