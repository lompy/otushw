#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/prandom.h>
#include <linux/rwlock.h>
#include <linux/slab.h>

#define NUM_READERS 3
#define NUM_WRITERS 2
#define NUM_ITERATIONS 5

static struct {
	rwlock_t lock;
	unsigned long data;
	struct task_struct *readers[NUM_READERS];
	struct completion reader_completions[NUM_READERS];
	struct task_struct *writers[NUM_WRITERS];
	struct completion writer_completions[NUM_READERS];
} ex_rwlock_data;

static int reader_function(void *data)
{
	unsigned long reader_id = (unsigned long)data;

	for (int i = 0; i < NUM_ITERATIONS; i++) {
		read_lock(&ex_rwlock_data.lock);
		pr_info("reader %lu: read data = %lu\n", reader_id,
			ex_rwlock_data.data);
		read_unlock(&ex_rwlock_data.lock);

		msleep(prandom_u32_max(100));
	}

	complete(&ex_rwlock_data.reader_completions[reader_id]);

	return 0;
}

static int writer_function(void *data)
{
	unsigned long writer_id = (unsigned long)data;

	for (int i = 0; i < NUM_ITERATIONS; ++i) {
		write_lock(&ex_rwlock_data.lock);
		ex_rwlock_data.data = writer_id * 100 + i;
		pr_info("writer %lu: wrote data = %lu\n", writer_id,
			ex_rwlock_data.data);
		write_unlock(&ex_rwlock_data.lock);
		msleep(prandom_u32_max(100));
	}

	complete(&ex_rwlock_data.writer_completions[writer_id]);

	return 0;
}

static int __init ex_rwlock_init(void)
{
	pr_info("loading\n");

	rwlock_init(&ex_rwlock_data.lock);
	ex_rwlock_data.data = 0;

	for (unsigned long i = 0; i < NUM_READERS; ++i) {
		init_completion(&ex_rwlock_data.reader_completions[i]);
		ex_rwlock_data.readers[i] = kthread_run(
			reader_function, (void *)i, "reader_thread_%lu", i);
		if (IS_ERR(ex_rwlock_data.readers[i])) {
			pr_err("kthread run for reader %lu: %ld\n", i,
			       PTR_ERR(ex_rwlock_data.readers[i]));
			return PTR_ERR(ex_rwlock_data.readers[i]);
		}
	}

	for (unsigned long i = 0; i < NUM_WRITERS; ++i) {
		init_completion(&ex_rwlock_data.writer_completions[i]);
		ex_rwlock_data.writers[i] = kthread_run(
			writer_function, (void *)i, "writer_thread_%lu", i);
		if (IS_ERR(ex_rwlock_data.writers[i])) {
			pr_err("kthread run for writer %lu: %ld\n", i,
			       PTR_ERR(ex_rwlock_data.writers[i]));
			return PTR_ERR(ex_rwlock_data.writers[i]);
		}
	}

	pr_info("loaded\n");

	return 0;
}

static void __exit ex_rwlock_exit(void)
{
	for (int i = 0; i < NUM_READERS; ++i)
		wait_for_completion(&ex_rwlock_data.reader_completions[i]);

	for (int i = 0; i < NUM_WRITERS; ++i)
		wait_for_completion(&ex_rwlock_data.writer_completions[i]);

	pr_info("unloaded, value: %lu\n", ex_rwlock_data.data);
}

module_init(ex_rwlock_init);
module_exit(ex_rwlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("Mutex example with multiple kthreads");
