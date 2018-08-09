#include <stdio.h>
#include <typedef.h>
#include <kdebug.h>
#include <board.h>
#include <kernel.h>
#include <stm32f4xx_conf.h>
#include <testcase.h>
#include <timer.h>

#define SYS_INIT_STACK_SIZE 1024
task_t sys_init;
static unsigned char sys_init_stack[SYS_INIT_STACK_SIZE];

#define TEST_TASK1_STACK_SIZE 1024
task_t test_task1;
static unsigned char test_task1_stack[SYS_INIT_STACK_SIZE];

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

extern int task_test(void);

static int test_task1_run(void *arg)
{
    int cnt = 0;

    while(1) {
        kdebug_print("%s cnt %d\r\n", __func__, cnt++);
        msleep(499);
    }
}

static int sys_init_run(void *arg)
{
    int ret;
    int cnt = 0;

    KDBG(INFO, "board_init\r\n");
    board_init();

    arch_systick_start();

    register_periodical_timer("timer1", 699, my_timer1, NULL);
    register_oneshot_timer("timer2",788, my_timer2, NULL);

    task_create(&test_task1,
                "test_task1",
                10,
                test_task1_run,
                NULL,
                test_task1_stack,
                TEST_TASK1_STACK_SIZE,
                5,
                0);
    task_resume(&test_task1);

   task_test();

    while(1) {
        kdebug_print("%s: sys_init loop %d\r\n", __func__, cnt++);
        msleep(1000);
        //task_yield();
    }

    return ret;
}

int os_start()
{
    arch_init();

    KDBG(INFO, "*** welcome to iotos *** \r\n\r\n");

    KDBG(INFO, "os_init\r\n");

    os_init();

    task_create(&sys_init,
                "sys_init",
                10,
                sys_init_run,
                NULL,
                sys_init_stack,
                SYS_INIT_STACK_SIZE,
                5,
                0);


    os_run();

    while (1) {
        KASSERT(0);
    }

    return 0;
}



