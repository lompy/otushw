#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/prandom.h>
#include <linux/semaphore.h>
#include <linux/slab.h>

#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 3
#define BUFFER_SIZE 5
#define NUM_ITEMS 10

static struct {
	struct semaphore mutex;
	struct semaphore empty;
	struct semaphore filled;

	unsigned long buffer[BUFFER_SIZE];
	size_t write_pos;
	size_t read_pos;

	struct task_struct *producers[NUM_PRODUCERS];
	struct completion producer_completions[NUM_PRODUCERS];
	struct task_struct *consumers[NUM_CONSUMERS];
	struct completion consumer_completions[NUM_CONSUMERS];
} ex_semaphore_data;

static int producer_function(void *data)
{
	unsigned long producer_id = (unsigned long)data;
	unsigned long item;
	int items_produced = 0;

	while (items_produced < NUM_ITEMS) {
		item = producer_id * 1000 + items_produced;

		down(&ex_semaphore_data.empty);

		down(&ex_semaphore_data.mutex);

		ex_semaphore_data.buffer[ex_semaphore_data.write_pos] = item;
		ex_semaphore_data.write_pos =
			(ex_semaphore_data.write_pos + 1) % BUFFER_SIZE;

		pr_info("producer %lu: produced item %lu\n", producer_id, item);

		up(&ex_semaphore_data.mutex);
		up(&ex_semaphore_data.filled);

		items_produced++;

		msleep(prandom_u32_max(100));
	}

	complete(&ex_semaphore_data.producer_completions[producer_id]);
	pr_info("producer %lu: completed\n", producer_id);

	return 0;
}

static int consumer_function(void *data)
{
	unsigned long consumer_id = (unsigned long)data;
	unsigned long item;

	while (true) {
		pr_info("consumer %lu: ready to consume\n", consumer_id);

		if (down_interruptible(&ex_semaphore_data.filled))
			break;
		down(&ex_semaphore_data.mutex);

		item = ex_semaphore_data.buffer[ex_semaphore_data.read_pos];
		ex_semaphore_data.read_pos =
			(ex_semaphore_data.read_pos + 1) % BUFFER_SIZE;

		pr_info("consumer %lu: consumed item %lu\n", consumer_id, item);

		up(&ex_semaphore_data.mutex);
		up(&ex_semaphore_data.empty);

		msleep(prandom_u32_max(150));
	}

	complete(&ex_semaphore_data.consumer_completions[consumer_id]);
	pr_info("consumer %lu: completed\n", consumer_id);

	return 0;
}

static int __init ex_semaphore_init(void)
{
	pr_info("loading\n");

	sema_init(&ex_semaphore_data.mutex, 1);
	sema_init(&ex_semaphore_data.empty, BUFFER_SIZE);
	sema_init(&ex_semaphore_data.filled, 0);
	ex_semaphore_data.write_pos = 0;
	ex_semaphore_data.read_pos = 0;

	for (unsigned long i = 0; i < NUM_PRODUCERS; ++i) {
		init_completion(&ex_semaphore_data.producer_completions[i]);
		ex_semaphore_data.producers[i] = kthread_run(
			producer_function, (void *)i, "producer_thread_%lu", i);
		if (IS_ERR(ex_semaphore_data.producers[i])) {
			pr_err("kthread run for producer %lu: %ld\n", i,
			       PTR_ERR(ex_semaphore_data.producers[i]));
			return PTR_ERR(ex_semaphore_data.producers[i]);
		}
	}

	for (unsigned long i = 0; i < NUM_CONSUMERS; ++i) {
		init_completion(&ex_semaphore_data.consumer_completions[i]);
		ex_semaphore_data.consumers[i] = kthread_run(
			consumer_function, (void *)i, "consumer_thread_%lu", i);
		if (IS_ERR(ex_semaphore_data.consumers[i])) {
			pr_err("kthread run for producer %lu: %ld\n", i,
			       PTR_ERR(ex_semaphore_data.consumers[i]));
			return PTR_ERR(ex_semaphore_data.consumers[i]);
		}
	}

	pr_info("loaded\n");

	return 0;
}

static void __exit ex_semaphore_exit(void)
{
	for (int i = 0; i < NUM_PRODUCERS; ++i)
		wait_for_completion(&ex_semaphore_data.producer_completions[i]);

	for (int i = 0; i < NUM_CONSUMERS; ++i)
		kthread_stop(ex_semaphore_data.consumers[i]);

	for (int i = 0; i < NUM_CONSUMERS; ++i)
		wait_for_completion(&ex_semaphore_data.consumer_completions[i]);

	pr_info("unloaded\n");
}

module_init(ex_semaphore_init);
module_exit(ex_semaphore_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("Mutex example with multiple kthreads");
