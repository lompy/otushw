#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/prandom.h>
#include <linux/slab.h>

#define NUM_THREADS 3
#define NUM_INCREMENTS 5

static struct {
	struct mutex mtx;
	int counter;
	struct task_struct *threads[NUM_THREADS];
	struct completion completions[NUM_THREADS];
} ex_mutex_data;

static int thread_function(void *data)
{
	unsigned long thread_id = (unsigned long)data;

	for (int i = 0; i < NUM_INCREMENTS; ++i) {
		mutex_lock(&ex_mutex_data.mtx);
		ex_mutex_data.counter++;
		pr_info("thread %ld: counter: %d\n", thread_id,
			ex_mutex_data.counter);
		mutex_unlock(&ex_mutex_data.mtx);

		msleep(prandom_u32_max(50));
	}

	complete(&ex_mutex_data.completions[thread_id]);

	return 0;
}

static int __init ex_mutex_init(void)
{
	mutex_init(&ex_mutex_data.mtx);
	ex_mutex_data.counter = 0;

	for (unsigned long i = 0; i < NUM_THREADS; i++) {
		init_completion(&ex_mutex_data.completions[i]);
		ex_mutex_data.threads[i] = kthread_run(
			thread_function, (void *)i, "ex_mutex_th_%lu", i);
		if (IS_ERR(ex_mutex_data.threads[i])) {
			pr_err("kthread run for thread %lu: %ld\n", i,
			       PTR_ERR(ex_mutex_data.threads[i]));

			return PTR_ERR(ex_mutex_data.threads[i]);
		}
	}

	pr_info("loaded\n");

	return 0;
}

static void __exit ex_mutex_exit(void)
{
	for (int i = 0; i < NUM_THREADS; i++)
		wait_for_completion(&ex_mutex_data.completions[i]);

	pr_info("unloaded, counter: %d\n", ex_mutex_data.counter);
}

module_init(ex_mutex_init);
module_exit(ex_mutex_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("Mutex example with multiple kthreads");
