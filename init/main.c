#include <typedef.h>
#include <kdebug.h>
#include <board.h>
#include <stm32f4xx_conf.h>
#include <testcase.h>
#include <task.h>
#include <timer.h>
#include <sem.h>
#include <ringbuf.h>
#include <workqueue.h>

extern void os_init(void);
extern void os_run(void);

#define SYS_INIT_STACK_SIZE 1024
task_t sys_init;
static unsigned char sys_init_stack[SYS_INIT_STACK_SIZE];

#define TEST_TASK1_STACK_SIZE 1024
task_t test_task1;
static unsigned char test_task1_stack[SYS_INIT_STACK_SIZE];

sem_t sem1;

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

ringbuf_t *ringbuf = NULL;

static void do_work1(void *arg)
{
    static int cnt = 0;
    worker_t *worker = (worker_t*)arg;

    kdebug_print("workqueue1 work cnt %d\r\n", cnt++);
    msleep(1000);
    workqueue_queue_worker(worker);
}

static void do_work2(void *arg)
{
    static int cnt = 0;
    worker_t *worker = (worker_t*)arg;

    kdebug_print("workqueue2 work cnt %d\r\n", cnt++);
    msleep(1010);
    workqueue_queue_worker(worker);
}


static  worker_t worker1, worker2;
static int test_task1_run(void *arg)
{
    int cnt = 0;
    workqueue_init_worker(&worker1, do_work1, &worker1, 1000);
    workqueue_init_worker(&worker2, do_work2, &worker2, 1010);
    workqueue_queue_worker(&worker1);
    workqueue_queue_worker(&worker2);

    while(1) {
        kdebug_print("%s cnt %d\r\n", __func__, cnt);
        msleep(2000);
        char data = (cnt%26) + 'a';
        ringbuf_queue(ringbuf, data);
        kdebug_print("ringbuf queue %c\r\n", data);
        sem_post(&sem1);

        cnt++;
    }
}

static int sys_init_run(void *arg)
{
    int ret;
    int cnt = 0;
    timer_t timer1, timer2;

    KINFO("board_init\r\n");
    board_init();

    arch_systick_start();

    register_periodical_timer(&timer1, "timer1", 5000, my_timer1, NULL);
    register_oneshot_timer(&timer2, "timer2",10000, my_timer2, NULL);

    sem_init(&sem1, 0);

    ringbuf = ringbuf_init(20);
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

    while(1) {
        kdebug_print("%s: sys_init loop %d\r\n", __func__, cnt);

        sem_wait(&sem1);
        char data = 'P';
        ringbuf_dequeue(ringbuf, &data);
        kdebug_print("read ringbuf: %c\r\n", data);

        cnt++;
    }

    return ret;
}

int os_start()
{
    arch_init();

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
                0);

    os_run();

    while (1) {
        KASSERT(0);
    }

    return 0;
}

