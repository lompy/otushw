#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#define MAX_LEN 64
#define CAP 4

struct message {
	char string[MAX_LEN];
};

struct ex_queue {
	DECLARE_KFIFO(fifo, struct message, CAP);
	struct mutex mtx;
};

extern struct kobject *kernel_kobj;
static struct kobject *ex_queue_kobj;
static struct ex_queue *mq;

static int put_msg(const struct message *msg)
{
	mutex_lock(&mq->mtx);
	if (kfifo_is_full(&mq->fifo)) {
		mutex_unlock(&mq->mtx);

		return -ENOSPC;
	}

	int ret = kfifo_put(&mq->fifo, *msg);
	mutex_unlock(&mq->mtx);

	return ret;
}

static int get_msg(struct message *msg)
{
	mutex_lock(&mq->mtx);
	if (kfifo_is_empty(&mq->fifo)) {
		mutex_unlock(&mq->mtx);

		return -EAGAIN;
	}

	int ret = kfifo_get(&mq->fifo, msg);
	mutex_unlock(&mq->mtx);

	return ret;
}

static ssize_t put_msg_kobjattr_store(struct kobject *_kobj,
				      struct kobj_attribute *_attr,
				      const char *buf, size_t count)
{
	size_t len = strcspn(buf, "\n");
	if (len <= 0 || len >= MAX_LEN) {
		pr_err("invalid input: string length must be positive and less than %d\n",
		       MAX_LEN);

		return -EINVAL;
	}

	struct message msg = { .string = { 0 } };
	memcpy(msg.string, buf, len);

	int ret = put_msg(&msg);
	if (ret < 0) {
		pr_err("get: %pe\n", ERR_PTR(ret));

		return ret;
	}

	pr_info("put: %s\n", msg.string);
	return count;
}

static ssize_t get_msg_kobjattr_show(struct kobject *_kobj,
				     struct kobj_attribute *_attr, char *buf)
{
	struct message msg = { .string = { 0 } };

	int ret = get_msg(&msg);
	if (ret < 0) {
		pr_err("get: %pe\n", ERR_PTR(ret));

		return ret;
	}

	ssize_t pos = scnprintf(buf, PAGE_SIZE, "%s\n", msg.string);
	if (pos == 0)
		pr_err("get: %pe\n", ERR_PTR(ret));

	pr_info("get: %s\n", msg.string);

	return pos;
}

static struct kobj_attribute put_msg_kobjattr =
	__ATTR(put, 0220, NULL, put_msg_kobjattr_store);
static struct kobj_attribute get_msg_kobjattr =
	__ATTR(get, 0444, get_msg_kobjattr_show, NULL);

static struct attribute *attrs[] = {
	&put_msg_kobjattr.attr,
	&get_msg_kobjattr.attr,
	NULL,
};

static struct attribute_group attr_group = { .attrs = attrs };

static struct ex_queue *mq; // Global pointer to our queue structure

static int __init ex_queue_init(void)
{
	mq = kmalloc(sizeof(*mq), GFP_KERNEL);
	if (!mq) {
		pr_err("kmalloc ex_queue: no memory\n");

		return -ENOMEM;
	}

	mutex_init(&mq->mtx);
	INIT_KFIFO(mq->fifo);

	ex_queue_kobj = kobject_create_and_add("ex_queue", kernel_kobj);
	if (!ex_queue_kobj) {
		pr_err("kobject_create_and_add: no memory");
		kfree(mq);

		return -ENOMEM;
	}

	int ret = sysfs_create_group(ex_queue_kobj, &attr_group);
	if (ret) {
		pr_err("sysfs_create_group: %pe\n", ERR_PTR(ret));
		kobject_put(ex_queue_kobj);
		kfree(mq);

		return ret;
	}

	pr_info("loaded, max string length: %u\n", MAX_LEN - 1);

	return 0;
}

static void __exit ex_queue_exit(void)
{
	if (ex_queue_kobj) {
		sysfs_remove_group(ex_queue_kobj, &attr_group);
		kobject_put(ex_queue_kobj);
	}

	if (mq)
		kfree(mq);

	pr_info("unloaded\n");
}

module_init(ex_queue_init);
module_exit(ex_queue_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("Module shows kfifo API usage controlled by sysfs");
