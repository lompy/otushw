#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/prandom.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 20

static struct {
	spinlock_t lock;
	unsigned long flags;
	int counter;
	struct task_struct *threads[NUM_THREADS];
	struct completion thread_completions[NUM_THREADS];
} ex_spinlock_data;

static int thread_function(void *data)
{
	unsigned long thread_id = (unsigned long)data;
	unsigned long flags;

	for (int i = 0; i < NUM_ITERATIONS; ++i) {
		if (thread_id % 2 == 0) {
			spin_lock(&ex_spinlock_data.lock);
			ex_spinlock_data.counter++;
			pr_info("thread %lu (regular spinlock): counter = %d\n",
				thread_id, ex_spinlock_data.counter);
			spin_unlock(&ex_spinlock_data.lock);
		} else {
			spin_lock_irqsave(&ex_spinlock_data.lock, flags);
			ex_spinlock_data.counter++;
			pr_info("thread %lu (irqsave spinlock): counter = %d\n",
				thread_id, ex_spinlock_data.counter);
			spin_unlock_irqrestore(&ex_spinlock_data.lock, flags);
		}

		msleep(prandom_u32_max(100));
	}

	complete(&ex_spinlock_data.thread_completions[thread_id]);
	pr_info("thread %lu: completed\n", thread_id);

	return 0;
}

static int __init ex_spinlock_init(void)
{
	pr_info("loading\n");

	spin_lock_init(&ex_spinlock_data.lock);
	ex_spinlock_data.counter = 0;

	for (unsigned long i = 0; i < NUM_THREADS; ++i) {
		init_completion(&ex_spinlock_data.thread_completions[i]);
		ex_spinlock_data.threads[i] = kthread_run(
			thread_function, (void *)i, "spinlock_thread_%lu", i);
		if (IS_ERR(ex_spinlock_data.threads[i])) {
			pr_err("kthread run for thread %lu: %ld\n", i,
			       PTR_ERR(ex_spinlock_data.threads[i]));
			return PTR_ERR(ex_spinlock_data.threads[i]);
		}
	}

	pr_info("loaded\n");

	return 0;
}

static void __exit ex_spinlock_exit(void)
{
	for (int i = 0; i < NUM_THREADS; ++i)
		wait_for_completion(&ex_spinlock_data.thread_completions[i]);

	pr_info("unloaded, counter: %d\n", ex_spinlock_data.counter);
}

module_init(ex_spinlock_init);
module_exit(ex_spinlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("Mutex example with multiple kthreads");
