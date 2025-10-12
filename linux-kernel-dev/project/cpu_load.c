#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cdev.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cpu_load"
#define CLASS_NAME "cpu_load"

#define CPU_LOAD_SET_INTERVAL _IOW('c', 1, int)

struct cpu_stat {
	unsigned long long user;
	unsigned long long nice;
	unsigned long long system;
	unsigned long long idle;
	unsigned long long iowait;
	unsigned long long irq;
	unsigned long long softirq;
	unsigned long long steal;
	unsigned long long guest;
	unsigned long long guest_nice;
};

struct load_sample {
	ktime_t timestamp;
	unsigned int cpu_id;
	unsigned long long total;
	unsigned long long idle;
	unsigned int load_percent;
};

struct cpu_buffer {
	struct load_sample *buffer;
	unsigned int head;
	unsigned int tail;
	unsigned int count;
};

struct cpu_load_dev {
	struct mutex lock;
	unsigned int interval_ms;
	struct timer_list timer;

	struct cpu_buffer __percpu *cpu_buffers;

	struct cdev cdev;
	struct class *dev_class;
	struct device *device;
	dev_t dev_num;
};

static int buffer_size = 60;
module_param(buffer_size, int, 0644);
MODULE_PARM_DESC(buffer_size, "size of the ring buffer");

static int sample_interval = 1000;
module_param(sample_interval, int, 0644);
MODULE_PARM_DESC(sample_interval, "sampling interval in milliseconds");

static struct cpu_load_dev *dev_data;

static void store_sample(struct load_sample *sample, int cpu)
{
	struct cpu_buffer *cpu_buf = per_cpu_ptr(dev_data->cpu_buffers, cpu);

	if (cpu_buf->count == buffer_size) {
		cpu_buf->tail = (cpu_buf->tail + 1) % buffer_size;
		cpu_buf->count--;
	}

	cpu_buf->buffer[cpu_buf->head] = *sample;
	cpu_buf->head = (cpu_buf->head + 1) % buffer_size;
	cpu_buf->count++;

	pr_info("after store CPU %d: head = %du, tail = %du, count = %du\n",
		cpu, cpu_buf->head, cpu_buf->tail, cpu_buf->count);
}

static void get_cpu_stats(struct cpu_stat *stat, int cpu)
{
	struct kernel_cpustat kcpustat;

	if (!cpu_online(cpu)) {
		memset(stat, 0, sizeof(*stat));
		return;
	}

	kcpustat = kcpustat_cpu(cpu);

	stat->user = kcpustat.cpustat[CPUTIME_USER];
	stat->nice = kcpustat.cpustat[CPUTIME_NICE];
	stat->system = kcpustat.cpustat[CPUTIME_SYSTEM];
	stat->idle = kcpustat.cpustat[CPUTIME_IDLE];
	stat->iowait = kcpustat.cpustat[CPUTIME_IOWAIT];
	stat->irq = kcpustat.cpustat[CPUTIME_IRQ];
	stat->softirq = kcpustat.cpustat[CPUTIME_SOFTIRQ];
	stat->steal = kcpustat.cpustat[CPUTIME_STEAL];
	stat->guest = kcpustat.cpustat[CPUTIME_GUEST];
	stat->guest_nice = kcpustat.cpustat[CPUTIME_GUEST_NICE];
}

static void cpu_load_timer_callback(struct timer_list *t)
{
	struct cpu_load_dev *dev = from_timer(dev, t, timer);
	int cpu;
	struct cpu_stat new_stat;
	struct load_sample sample;
	unsigned long long total_new, idle_new;

	for_each_possible_cpu(cpu) {
		pr_info("get_cpu_stats for %d\n", cpu);
		get_cpu_stats(&new_stat, cpu);

		total_new = new_stat.user + new_stat.nice + new_stat.system +
			    new_stat.idle + new_stat.iowait + new_stat.irq +
			    new_stat.softirq + new_stat.steal;
		idle_new = new_stat.idle + new_stat.iowait;

		pr_info("stats for %d: total %llu, idle %llu\n", cpu, total_new,
			idle_new);

		sample.timestamp = ktime_get();
		sample.cpu_id = cpu;
		sample.total = total_new;
		sample.idle = idle_new;

		mutex_lock(&dev->lock);
		store_sample(&sample, cpu);
		mutex_unlock(&dev->lock);

		pr_info("CPU %d: total=%llu idle=%llu\n", cpu, total_new,
			idle_new);
	}

	mod_timer(&dev->timer, jiffies + msecs_to_jiffies(dev->interval_ms));
}

static int cpu_load_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int cpu_load_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t cpu_load_read(struct file *file, char __user *buf, size_t count,
			     loff_t *offset)
{
	char *kbuf;
	size_t len = 0;
	int cpu;
	struct load_sample sample;
	struct cpu_buffer *cpu_buffer;
	bool has_data = false;

	kbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;

	mutex_lock(&dev_data->lock);

	// First check if we have any data
	for_each_possible_cpu(cpu) {
		cpu_buffer = per_cpu_ptr(dev_data->cpu_buffers, cpu);
		if (cpu_buffer->count > 0) {
			has_data = true;
			break;
		}
	}

	if (!has_data) {
		mutex_unlock(&dev_data->lock);
		kfree(kbuf);
		return -EAGAIN; // Tell user to try again
	}

	for_each_possible_cpu(cpu) {
		cpu_buffer = per_cpu_ptr(dev_data->cpu_buffers, cpu);
		if (cpu_buffer->count > 0) {
			// Get latest sample safely
			unsigned int idx = (cpu_buffer->head > 0) ?
						   (cpu_buffer->head - 1) :
						   (buffer_size - 1);
			sample = cpu_buffer->buffer[idx];
			len += scnprintf(kbuf + len, PAGE_SIZE - len,
					 "%d,%llu,%llu\n", cpu, sample.total,
					 sample.idle);
		}
	}

	mutex_unlock(&dev_data->lock);

	if (copy_to_user(buf, kbuf, len)) {
		kfree(kbuf);
		return -EFAULT;
	}

	kfree(kbuf);
	return len;
}

static long cpu_load_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{
	int new_interval;

	switch (cmd) {
	case CPU_LOAD_SET_INTERVAL:
		if (copy_from_user(&new_interval, (int *)arg, sizeof(int)))
			return -EFAULT;

		if (new_interval < 100 || new_interval > 10000)
			return -EINVAL;

		mutex_lock(&dev_data->lock);
		dev_data->interval_ms = new_interval;
		mutex_unlock(&dev_data->lock);

		mod_timer(&dev_data->timer,
			  jiffies + msecs_to_jiffies(dev_data->interval_ms));
		break;

	default:
		return -ENOTTY;
	}

	return 0;
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
	int ret, cpu;

	dev_data = kzalloc(sizeof(*dev_data), GFP_KERNEL);
	if (!dev_data)
		return -ENOMEM;

	mutex_init(&dev_data->lock);

	dev_data->cpu_buffers = alloc_percpu(typeof(*dev_data->cpu_buffers));
	if (!dev_data->cpu_buffers) {
		ret = -ENOMEM;

		goto fail_alloc_buffers;
	}

	for_each_possible_cpu(cpu) {
		struct cpu_buffer *cpu_buf =
			per_cpu_ptr(dev_data->cpu_buffers, cpu);

		cpu_buf->buffer = kzalloc(
			sizeof(struct load_sample) * buffer_size, GFP_KERNEL);
		if (!cpu_buf->buffer) {
			ret = -ENOMEM;

			goto fail_alloc_cpu_buffer;
		}
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
	ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
	if (ret < 0)
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
fail_alloc_cpu_buffer:
	for_each_possible_cpu(cpu) {
		struct cpu_buffer *cpu_buf =
			per_cpu_ptr(dev_data->cpu_buffers, cpu);
		kfree(cpu_buf->buffer);
	}
	free_percpu(dev_data->cpu_buffers);
fail_alloc_buffers:
	kfree(dev_data);

	return ret;
}

static void __exit cpu_load_exit(void)
{
	int cpu;

	del_timer_sync(&dev_data->timer);

	cdev_del(&dev_data->cdev);
	device_destroy(dev_data->dev_class, dev_data->dev_num);
	class_destroy(dev_data->dev_class);
	unregister_chrdev_region(dev_data->dev_num, 1);

	for_each_possible_cpu(cpu) {
		struct cpu_buffer *cpu_buf =
			per_cpu_ptr(dev_data->cpu_buffers, cpu);
		kfree(cpu_buf->buffer);
	}
	free_percpu(dev_data->cpu_buffers);

	kfree(dev_data);

	pr_info("unloaded\n");
}

module_init(cpu_load_init);
module_exit(cpu_load_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("CPU Load Monitor Device");
MODULE_VERSION("0.1");
