#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>

#define MAX_LEN (64 - sizeof(struct list_head))

struct string_entry {
	struct list_head nodes;
	char string[MAX_LEN];
};

static LIST_HEAD(ex_list);
static DEFINE_MUTEX(ex_list_mtx);

extern struct kobject *kernel_kobj;
static struct kobject *ex_list_kobj;

static int add_entry(const char *string)
{
	if (strnlen(string, MAX_LEN) >= MAX_LEN) {
		pr_err("invalid input: string length must be less than %lu\n",
		       MAX_LEN);

		return -EINVAL;
	}

	struct string_entry *entry = kmalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry) {
		pr_err("no memory: unable to kmalloc string_entry");

		return -ENOMEM;
	}

	strncpy(entry->string, string, MAX_LEN);

	mutex_lock(&ex_list_mtx);
	list_add_tail(&entry->nodes, &ex_list);
	mutex_unlock(&ex_list_mtx);

	pr_info("added: %s\n", string);

	return 0;
}

static void del_entry(const char *string)
{
	struct string_entry *pos, *_tmp;

	mutex_lock(&ex_list_mtx);
	list_for_each_entry_safe(pos, _tmp, &ex_list, nodes) {
		if (strncmp(pos->string, string, MAX_LEN) == 0) {
			list_del(&pos->nodes);
			kfree(pos);
			pr_info("removed: %s\n", string);
			break;
		}
	}
	mutex_unlock(&ex_list_mtx);
}

static ssize_t add_entry_kobjattr_store(struct kobject *_kobj,
					struct kobj_attribute *_attr,
					const char *buf, size_t count)
{
	size_t len = strcspn(buf, "\n");
	if (len >= MAX_LEN) {
		pr_err("invalid input: string length must be less than %lu\n",
		       MAX_LEN);

		return -EINVAL;
	}

	char new_string[MAX_LEN] = { 0 };
	memcpy(new_string, buf, len);

	int ret = add_entry(new_string);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t del_entry_kobjattr_store(struct kobject *_kobj,
					struct kobj_attribute *_attr,
					const char *buf, size_t count)
{
	size_t len = strcspn(buf, "\n");
	if (len >= MAX_LEN) {
		pr_err("invalid input: string length must be less than %lu\n",
		       MAX_LEN);

		return -EINVAL;
	}

	char string[MAX_LEN] = { 0 };
	memcpy(string, buf, len);

	del_entry(string);

	return count;
}

static ssize_t entries_kobjattr_show(struct kobject *_kobj,
				     struct kobj_attribute *_attr, char *buf)
{
	struct string_entry *entry;
	ssize_t pos = 0;

	mutex_lock(&ex_list_mtx);
	list_for_each_entry(entry, &ex_list, nodes) {
		pos += scnprintf(buf + pos, PAGE_SIZE - pos, "%s\n",
				 entry->string);
		if (pos >= PAGE_SIZE)
			break;
	}
	mutex_unlock(&ex_list_mtx);

	return pos;
}

static struct kobj_attribute add_entry_kobjattr =
	__ATTR(add_entry, 0220, NULL, add_entry_kobjattr_store);
static struct kobj_attribute del_entry_kobjattr =
	__ATTR(del_entry, 0220, NULL, del_entry_kobjattr_store);
static struct kobj_attribute entries_kobjattr =
	__ATTR(entries, 0444, entries_kobjattr_show, NULL);

static struct attribute *attrs[] = {
	&add_entry_kobjattr.attr,
	&del_entry_kobjattr.attr,
	&entries_kobjattr.attr,
	NULL,
};

static struct attribute_group attr_group = { .attrs = attrs };

static int __init ex_list_init(void)
{
	int ret;

	ex_list_kobj = kobject_create_and_add("ex_list", kernel_kobj);
	if (!ex_list_kobj) {
		pr_err("no memory: unable to kobject_create_and_add");

		return -ENOMEM;
	}

	ret = sysfs_create_group(ex_list_kobj, &attr_group);
	if (ret) {
		kobject_put(ex_list_kobj);

		return ret;
	}

	pr_info("loaded, max string length: %lu\n", MAX_LEN - 1);

	return 0;
}

static void __exit ex_list_exit(void)
{
	if (ex_list_kobj) {
		sysfs_remove_group(ex_list_kobj, &attr_group);
		kobject_put(ex_list_kobj);
	}

	struct string_entry *entry, *_tmp;
	mutex_lock(&ex_list_mtx);
	list_for_each_entry_safe(entry, _tmp, &ex_list, nodes) {
		list_del(&entry->nodes);
		kfree(entry);
	}
	mutex_unlock(&ex_list_mtx);

	pr_info("unloaded\n");
}

module_init(ex_list_init);
module_exit(ex_list_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION("Module shows list_head API usage with a simple string list "
		   "controlled by sysfs");
