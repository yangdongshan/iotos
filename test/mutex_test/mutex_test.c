#include <task.h>
#include <mutex.h>
#include <err.h>
#include <kdebug.h>

static char test_task_stack[1024];
static task_t test_task;
mutex_t mutex;


int test_task_entry(void *arg)
{
	int ret;

	kdebug_print("%s wait mutex...\r\n", __func__);
	ret = mutex_lock(&mutex);
	KASSERT(ret == ERR_OK);
	kdebug_print("%s wait mutex... done\r\n", __func__);

	kdebug_print("%s sleep 10 ticks...\r\n", __func__);
	ret = task_sleep(10);
	KASSERT(ret == ERR_OK);
	kdebug_print("%s sleep 10 ticks... done\r\n", __func__);

	kdebug_print("%s unlock mutex...\r\n", __func__);
	ret = mutex_lock(&mutex);
	KASSERT(ret == -ERR_MUTEX_NESTED);
	kdebug_print("%s unlock mutex... done\r\n", __func__);

	while(1);
}
static void mutex_test_01(void)
{
	int ret;
	mutex_t mutex;

    ret = mutex_init(&mutex);
	KASSERT(ret == ERR_OK);

	ret = mutex_lock(&mutex);
	KASSERT(ret == ERR_OK);

	ret = mutex_unlock(&mutex);
	KASSERT(ret == ERR_OK);
}

static void mutex_test_02(void)
{
	int ret;

    ret = mutex_init(&mutex);
	KASSERT(ret == ERR_OK);

	task_create(&test_task, "test_task", 15, test_task_entry, (void*)&mutex,
		        test_task_stack, 1024, 5, TF_AUTO_RUN);

	kdebug_print("%s wait mutex...\r\n", __func__);
	ret = mutex_lock(&mutex);
	KASSERT(ret == ERR_OK);
	kdebug_print("%s wait mutex... done\r\n", __func__);

	kdebug_print("%s sleep 10 ticks...\r\n", __func__);
	ret = task_sleep(10);
	KASSERT(ret == ERR_OK);
	kdebug_print("%s sleep 10 ticks... done\r\n", __func__);

	kdebug_print("%s unlock mutex...\r\n", __func__);
	ret = mutex_unlock(&mutex);
	KASSERT(ret == ERR_OK);
	kdebug_print("%s unlock mutex... done\r\n", __func__);

}

void mutex_test(void)
{
	mutex_test_01();
	mutex_test_02();
}
