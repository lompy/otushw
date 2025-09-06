#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/rbtree.h>
#include <linux/slab.h>

#define MAX_LEN (64 - sizeof(struct rb_node))

struct string_entry {
	struct rb_node node;
	char string[MAX_LEN];
};

static struct rb_root ex_rb_tree_root = RB_ROOT;

static DEFINE_MUTEX(ex_rb_tree_mtx);

extern struct kobject *kernel_kobj;
static struct kobject *ex_rb_tree_kobj;

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

	struct rb_node **new = &ex_rb_tree_root.rb_node, *parent = NULL;
	mutex_lock(&ex_rb_tree_mtx);
	while (*new) {
		struct string_entry *this =
			rb_entry(*new, struct string_entry, node);
		int result = strncmp(entry->string, this->string, MAX_LEN);
		parent = *new;

		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else
			return -EEXIST;
	}

	rb_link_node(&entry->node, parent, new);
	rb_insert_color(&entry->node, &ex_rb_tree_root);
	mutex_unlock(&ex_rb_tree_mtx);

	pr_info("added: %s\n", string);

	return 0;
}

static void del_entry(const char *string)
{
	struct rb_node *next = ex_rb_tree_root.rb_node;

	mutex_lock(&ex_rb_tree_mtx);
	while (next) {
		struct string_entry *this =
			rb_entry(next, struct string_entry, node);
		int result = strncmp(string, this->string, MAX_LEN);

		if (result < 0)
			next = next->rb_left;
		else if (result > 0)
			next = next->rb_right;
		else {
			next = NULL;
			rb_erase(&this->node, &ex_rb_tree_root);
			kfree(this);
			pr_info("removed: %s\n", string);
		}
	}
	mutex_unlock(&ex_rb_tree_mtx);
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
	mutex_lock(&ex_rb_tree_mtx);
	for (struct rb_node *node = rb_first(&ex_rb_tree_root); node;
	     node = rb_next(node)) {
		entry = rb_entry(node, struct string_entry, node);
		pos += scnprintf(buf + pos, PAGE_SIZE - pos, "%s\n",
				 entry->string);
		if (pos >= PAGE_SIZE)
			break;
	}
	mutex_unlock(&ex_rb_tree_mtx);

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

static int __init ex_rb_tree_init(void)
{
	int ret;

	ex_rb_tree_kobj = kobject_create_and_add("ex_rb_tree", kernel_kobj);
	if (!ex_rb_tree_kobj) {
		pr_err("no memory: unable to kobject_create_and_add");

		return -ENOMEM;
	}

	ret = sysfs_create_group(ex_rb_tree_kobj, &attr_group);
	if (ret) {
		kobject_put(ex_rb_tree_kobj);

		return ret;
	}

	pr_info("loaded, max string length: %lu\n", MAX_LEN - 1);

	return 0;
}

static void __exit ex_rb_tree_exit(void)
{
	if (ex_rb_tree_kobj) {
		sysfs_remove_group(ex_rb_tree_kobj, &attr_group);
		kobject_put(ex_rb_tree_kobj);
	}

	struct string_entry *entry, *_tmp;
	mutex_lock(&ex_rb_tree_mtx);
	rbtree_postorder_for_each_entry_safe(entry, _tmp, &ex_rb_tree_root,
					     node) {
		rb_erase(&entry->node, &ex_rb_tree_root);
		kfree(entry);
	}
	mutex_unlock(&ex_rb_tree_mtx);

	pr_info("unloaded\n");
}

module_init(ex_rb_tree_init);
module_exit(ex_rb_tree_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION(
	"Module shows rbtree API usage with string values controlled by sysfs");
