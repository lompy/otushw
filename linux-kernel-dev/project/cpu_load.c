#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cdev.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/rwlock.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define DEVICE_NAME "cpu_load"
#define CLASS_NAME "cpu_load"

#define MAX_SUPPORTED_CPUS 128
#define CPU_LOAD_SET_INTERVAL _IOW('c', 1, int)

#define MIN_BUFFER_SIZE 64
#define MAX_BUFFER_SIZE 1024
#define MIN_INTERVAL_MS 100
#define MAX_INTERVAL_MS (60 * 1000)
#define READER_BUF_SIZE PAGE_SIZE

static int buffer_size = MIN_BUFFER_SIZE;
module_param(buffer_size, int, 0444);
MODULE_PARM_DESC(buffer_size, "size of the ring buffer (64-1024 entries)");

static unsigned int sample_interval = 1000;
module_param(sample_interval, uint, 0444);
MODULE_PARM_DESC(sample_interval, "sampling interval in ms (100-60000 ms)");

struct cpu_load_stat {
	unsigned long long total;
	unsigned long long idle;
};

struct cpu_load_ring_buffer {
	struct {
		unsigned int *sample;
		ktime_t timestamp;
	} * buffer;
	unsigned int *temp_sample;
	unsigned int head;
	unsigned int tail;
	unsigned int count;
	unsigned long gen;
};

struct cpu_load_dev {
	unsigned int interval_ms;
	struct timer_list timer;

	rwlock_t lock;
	unsigned int num_cpus;
	int *cpu_ids;
	struct cpu_load_ring_buffer ring;

	spinlock_t readers_lock;
	struct list_head readers;
	wait_queue_head_t wait_queue;

	struct cdev cdev;
	struct class *dev_class;
	struct device *device;
	dev_t dev_num;

	atomic_t exiting;
};

struct cpu_load_reader {
	char *buf;
	size_t buf_size;
	size_t pos;
	size_t len;
	unsigned int last_ring_pos;
	unsigned long last_ring_gen;
	bool has_unread_data;

	struct list_head list;
};

static struct cpu_load_dev *dev_data;
static struct cpu_load_stat *prev_stats;

static void get_stats(struct cpu_load_stat *stat, int cpu)
{
	struct kernel_cpustat kcpustat = kcpustat_cpu(cpu);

	stat->total = kcpustat.cpustat[CPUTIME_USER] +
		      kcpustat.cpustat[CPUTIME_NICE] +
		      kcpustat.cpustat[CPUTIME_SYSTEM] +
		      kcpustat.cpustat[CPUTIME_IDLE] +
		      kcpustat.cpustat[CPUTIME_IOWAIT] +
		      kcpustat.cpustat[CPUTIME_IRQ] +
		      kcpustat.cpustat[CPUTIME_SOFTIRQ] +
		      kcpustat.cpustat[CPUTIME_STEAL];
	stat->idle = kcpustat.cpustat[CPUTIME_IDLE] +
		     kcpustat.cpustat[CPUTIME_IOWAIT];
}

static void save_temp_sample(void)
{
	memcpy(dev_data->ring.buffer[dev_data->ring.head].sample,
	       dev_data->ring.temp_sample,
	       dev_data->num_cpus * sizeof(unsigned int));

	dev_data->ring.buffer[dev_data->ring.head].timestamp = ktime_get_real();

	dev_data->ring.head = (dev_data->ring.head + 1) % buffer_size;
	if (dev_data->ring.head == 0)
		dev_data->ring.gen++;

	if (dev_data->ring.count == buffer_size)
		dev_data->ring.tail = dev_data->ring.head;
	else
		dev_data->ring.count++;

	pr_debug(
		"after store: head = %du, tail = %du, count = %du, gen = %lu\n",
		dev_data->ring.head, dev_data->ring.tail, dev_data->ring.count,
		dev_data->ring.gen);
}

static void cpu_load_timer_callback(struct timer_list *t)
{
	struct cpu_load_stat new_stat;
	for (size_t idx = 0; idx < dev_data->num_cpus; ++idx) {
		int cpu = dev_data->cpu_ids[idx];
		if (!cpu_online(cpu)) {
			pr_err("cpu %d went offline, reload the module!\n",
			       cpu);

			atomic_set(&dev_data->exiting, 1);
			del_timer(&dev_data->timer);
			wake_up_interruptible(&dev_data->wait_queue);

			return;
		}

		get_stats(&new_stat, cpu);

		unsigned long long total_delta =
			new_stat.total - prev_stats[idx].total;
		unsigned long long idle_delta =
			new_stat.idle - prev_stats[idx].idle;

		if (total_delta > 0)
			dev_data->ring.temp_sample[idx] =
				(100 * (total_delta - idle_delta)) /
				total_delta;
		else
			dev_data->ring.temp_sample[idx] = 0;

		prev_stats[idx].total = new_stat.total;
		prev_stats[idx].idle = new_stat.idle;

		pr_debug("cpu %d:%u%%\n", cpu, dev_data->ring.temp_sample[idx]);
	}

	write_lock(&dev_data->lock);
	save_temp_sample();
	write_unlock(&dev_data->lock);

	{
		spin_lock(&dev_data->readers_lock);
		struct cpu_load_reader *reader;
		list_for_each_entry(reader, &dev_data->readers, list) {
			reader->has_unread_data = true;
		}
		spin_unlock(&dev_data->readers_lock);
	}

	wake_up_interruptible(&dev_data->wait_queue);

	mod_timer(&dev_data->timer,
		  jiffies + msecs_to_jiffies(dev_data->interval_ms));
}

static int cpu_load_open(struct inode *inode, struct file *file)
{
	pr_info("opening\n");
	struct cpu_load_reader *reader;

	if (!try_module_get(THIS_MODULE))
		return -EBUSY;

	reader = kzalloc(sizeof(*reader), GFP_KERNEL);
	if (!reader) {
		module_put(THIS_MODULE);

		return -ENOMEM;
	}

	reader->buf_size = READER_BUF_SIZE;
	reader->buf = kmalloc(reader->buf_size, GFP_KERNEL);
	if (!reader->buf) {
		kfree(reader);
		module_put(THIS_MODULE);

		return -ENOMEM;
	}

	reader->pos = 0;
	reader->len = 0;
	reader->last_ring_gen = 0;
	reader->last_ring_pos = 0;
	INIT_LIST_HEAD(&reader->list);

	spin_lock(&dev_data->readers_lock);
	list_add(&reader->list, &dev_data->readers);
	spin_unlock(&dev_data->readers_lock);

	read_lock(&dev_data->lock);
	reader->has_unread_data = dev_data->ring.count > 0;
	read_unlock(&dev_data->lock);

	file->private_data = reader;
	pr_info("opened\n");

	return 0;
}

static int cpu_load_release(struct inode *inode, struct file *file)
{
	pr_info("closing\n");
	struct cpu_load_reader *reader = file->private_data;
	if (!reader)
		return 0;

	spin_lock(&dev_data->readers_lock);
	list_del(&reader->list);
	spin_unlock(&dev_data->readers_lock);

	kfree(reader->buf);
	kfree(reader);
	module_put(THIS_MODULE);
	pr_info("closed\n");

	return 0;
}

static ssize_t copy_reader_buf_to_user(struct cpu_load_reader *reader,
				       char __user *buf, size_t count)
{
	if (reader->len == 0 || reader->pos >= reader->len || count == 0)
		return 0;

	size_t to_copy = min(count, reader->len - reader->pos);
	if (copy_to_user(buf, reader->buf + reader->pos, to_copy))
		return -EFAULT;

	reader->pos += to_copy;

	return to_copy;
};

static ssize_t cpu_load_read(struct file *file, char __user *buf, size_t count,
			     loff_t *offset)
{
	if (atomic_read(&dev_data->exiting))
		return 0;

	struct cpu_load_reader *reader = file->private_data;
	if (!reader)
		return -EINVAL;

	if (reader->pos < reader->len)
		return copy_reader_buf_to_user(reader, buf, count);

	if (file->f_flags & O_NONBLOCK && !reader->has_unread_data)
		return -EAGAIN;

	int ret = wait_event_interruptible(
		dev_data->wait_queue,
		reader->has_unread_data || atomic_read(&dev_data->exiting));
	if (ret < 0)
		return ret;

	if (atomic_read(&dev_data->exiting))
		return 0;

	reader->pos = 0;
	reader->len = 0;
	reader->has_unread_data = false;
	read_lock(&dev_data->lock);

	if ((dev_data->ring.gen - reader->last_ring_gen > 1) ||
	    ((dev_data->ring.gen - reader->last_ring_gen == 1) &&
	     (dev_data->ring.head >= reader->last_ring_pos))) {
		reader->last_ring_pos = dev_data->ring.tail;
		reader->last_ring_gen = dev_data->ring.gen - 1;
	}

	do {
		struct timespec64 ts = ktime_to_timespec64(
			dev_data->ring.buffer[reader->last_ring_pos].timestamp);
		reader->len += scnprintf(reader->buf + reader->len,
					 reader->buf_size - reader->len,
					 "%lld.%03ld", ts.tv_sec,
					 ts.tv_nsec / NSEC_PER_MSEC);

		for (size_t idx = 0; idx < dev_data->num_cpus; ++idx)
			reader->len += scnprintf(
				reader->buf + reader->len,
				reader->buf_size - reader->len, " %2d:%03u",
				dev_data->cpu_ids[idx],
				dev_data->ring.buffer[reader->last_ring_pos]
					.sample[idx]);

		if (reader->len < reader->buf_size - 1) {
			reader->buf[reader->len++] = '\n';
		} else {
			pr_err("reader buffer overflow");

			return -EOVERFLOW;
		}

		reader->last_ring_pos++;
		if (reader->last_ring_pos == buffer_size) {
			reader->last_ring_pos = 0;
			reader->last_ring_gen++;
		}
	} while (reader->last_ring_pos != dev_data->ring.head);

	read_unlock(&dev_data->lock);

	return copy_reader_buf_to_user(reader, buf, count);
}

static long cpu_load_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{
	int new_interval;

	switch (cmd) {
	case CPU_LOAD_SET_INTERVAL:
		if (copy_from_user(&new_interval, (int *)arg, sizeof(int)))
			return -EFAULT;

		if (new_interval < MIN_INTERVAL_MS ||
		    new_interval > MAX_INTERVAL_MS) {
			pr_err("Invalid interval: %d (must be between %u and %u)\n",
			       new_interval, MIN_INTERVAL_MS, MAX_INTERVAL_MS);
			return -EINVAL;
		}

		write_lock(&dev_data->lock);
		dev_data->interval_ms = new_interval;
		write_unlock(&dev_data->lock);

		return 0;
	default:
		return -ENOTTY;
	}
}

static const struct file_operations cpu_load_fops = {
	.owner = THIS_MODULE,
	.open = cpu_load_open,
	.release = cpu_load_release,
	.read = cpu_load_read,
	.unlocked_ioctl = cpu_load_ioctl,
};

static int __init cpu_load_init(void)
{
	pr_info("loading\n");
	int ret = 0;

	if (buffer_size < MIN_BUFFER_SIZE || buffer_size > MAX_BUFFER_SIZE) {
		pr_err("buffer_size must be between %d and %d, got: %d\n",
		       MIN_BUFFER_SIZE, MAX_BUFFER_SIZE, buffer_size);
		return -EINVAL;
	}

	if (sample_interval < MIN_INTERVAL_MS ||
	    sample_interval > MAX_INTERVAL_MS) {
		pr_err("sample_interval must be between %u and %u, got: %u\n",
		       MIN_INTERVAL_MS, MAX_INTERVAL_MS, sample_interval);
		return -EINVAL;
	}

	dev_data = kzalloc(sizeof(*dev_data), GFP_KERNEL);
	if (!dev_data)
		return -ENOMEM;

	dev_data->num_cpus = num_online_cpus();

	prev_stats = kzalloc(sizeof(struct cpu_load_stat) * dev_data->num_cpus,
			     GFP_KERNEL);
	if (!prev_stats) {
		ret = -ENOMEM;

		goto fail_prev_stats;
	}

	dev_data->cpu_ids =
		kzalloc(sizeof(int) * dev_data->num_cpus, GFP_KERNEL);
	if (!dev_data->cpu_ids) {
		ret = -ENOMEM;

		goto fail_cpu_ids;
	}

	size_t idx = 0;
	int cpu;
	for_each_online_cpu(cpu) {
		if (idx >= dev_data->num_cpus) {
			pr_err("cpu count out of num_online_cpus range");
			ret = -EINVAL;

			goto fail_load_prev_stats;
		}

		get_stats(&prev_stats[idx], cpu);
		dev_data->cpu_ids[idx] = cpu;

		idx++;
	}

	rwlock_init(&dev_data->lock);
	spin_lock_init(&dev_data->readers_lock);
	INIT_LIST_HEAD(&dev_data->readers);
	init_waitqueue_head(&dev_data->wait_queue);
	atomic_set(&dev_data->exiting, 0);

	dev_data->ring.head = 0;
	dev_data->ring.tail = 0;
	dev_data->ring.count = 0;
	dev_data->ring.gen = 0;

	dev_data->ring.buffer = kzalloc(
		sizeof(*dev_data->ring.buffer) * buffer_size, GFP_KERNEL);
	if (!dev_data->ring.buffer) {
		ret = -ENOMEM;

		goto fail_alloc_ring_buffer;
	}

	for (int i = 0; i < buffer_size; i++) {
		dev_data->ring.buffer[i].sample = kzalloc(
			sizeof(unsigned int) * dev_data->num_cpus, GFP_KERNEL);
		if (!dev_data->ring.buffer[i].sample) {
			ret = -ENOMEM;

			goto fail_alloc_samples;
		}
	}

	dev_data->ring.temp_sample =
		kmalloc(sizeof(unsigned int) * dev_data->num_cpus, GFP_KERNEL);
	if (!dev_data->ring.temp_sample) {
		ret = -ENOMEM;
		goto fail_temp_sample;
	}

	ret = alloc_chrdev_region(&dev_data->dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0)
		goto fail_alloc_chrdev;

	dev_data->dev_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(dev_data->dev_class)) {
		ret = PTR_ERR(dev_data->dev_class);

		goto fail_class_create;
	}

	dev_data->device = device_create(dev_data->dev_class, NULL,
					 dev_data->dev_num, NULL, DEVICE_NAME);
	if (IS_ERR(dev_data->device)) {
		ret = PTR_ERR(dev_data->device);

		goto fail_device_create;
	}

	cdev_init(&dev_data->cdev, &cpu_load_fops);
	dev_data->cdev.owner = THIS_MODULE;
	if ((ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1)) < 0)
		goto fail_cdev_add;

	dev_data->interval_ms = sample_interval;
	timer_setup(&dev_data->timer, cpu_load_timer_callback, 0);
	mod_timer(&dev_data->timer,
		  jiffies + msecs_to_jiffies(dev_data->interval_ms));

	pr_info("loaded\n");

	return 0;

fail_cdev_add:
	device_destroy(dev_data->dev_class, dev_data->dev_num);
fail_device_create:
	class_destroy(dev_data->dev_class);
fail_class_create:
	unregister_chrdev_region(dev_data->dev_num, 1);
fail_alloc_chrdev:
	kfree(dev_data->ring.temp_sample);
fail_temp_sample:
	for (int i = 0; i < buffer_size; i++)
		if (dev_data->ring.buffer[i].sample)
			kfree(dev_data->ring.buffer[i].sample);
fail_alloc_samples:
	kfree(dev_data->ring.buffer);
fail_alloc_ring_buffer:
fail_load_prev_stats:
	kfree(dev_data->cpu_ids);
fail_cpu_ids:
	kfree(prev_stats);
fail_prev_stats:
	kfree(dev_data);
	return ret;
}

static void __exit cpu_load_exit(void)
{
	pr_info("unloading\n");

	del_timer_sync(&dev_data->timer);
	atomic_set(&dev_data->exiting, 1);
	wake_up_interruptible(&dev_data->wait_queue);

	cdev_del(&dev_data->cdev);
	device_destroy(dev_data->dev_class, dev_data->dev_num);
	class_destroy(dev_data->dev_class);
	unregister_chrdev_region(dev_data->dev_num, 1);

	spin_lock(&dev_data->readers_lock);
	struct cpu_load_reader *reader, *tmp_reader;
	list_for_each_entry_safe(reader, tmp_reader, &dev_data->readers, list) {
		list_del(&reader->list);
		kfree(reader->buf);
		kfree(reader);
	}
	spin_unlock(&dev_data->readers_lock);

	kfree(dev_data->ring.temp_sample);
	for (int i = 0; i < buffer_size; i++)
		kfree(dev_data->ring.buffer[i].sample);
	kfree(dev_data->ring.buffer);
	kfree(dev_data->cpu_ids);
	kfree(prev_stats);
	kfree(dev_data);

	pr_info("unloaded\n");
}

module_init(cpu_load_init);
module_exit(cpu_load_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("CPU Load Monitor Device (hotplug not supported)");
MODULE_VERSION("0.1");
