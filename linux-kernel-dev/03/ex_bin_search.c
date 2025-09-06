#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/bsearch.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sort.h>

#define MAX_SEARCH_SIZE 100000000

struct mod_size {
	const char *name;
	unsigned int size;
};

// just something random more complex than int to compare
static struct mod_size modules[] = {
	{ "ext4", 450000 },	 { "btrfs", 1200000 }, { "nvidia", 15000000 },
	{ "snd_hda", 120000 },	 { "usbhid", 50000 },  { "e1000", 180000 },
	{ "bluetooth", 650000 }, { "zfs", 3500000 },
};

static int search_size = 650000;

static const int num_modules = ARRAY_SIZE(modules);

static int mod_size_cmp(const void *a, const void *b)
{
	const struct mod_size *m1 = a, *m2 = b;

	if (m1->size < m2->size)
		return -1;
	if (m1->size > m2->size)
		return 1;

	return 0;
}

static int search_size_param_set(const char *val, const struct kernel_param *kp)
{
	int parsed_val;
	int parse_err = kstrtoint(val, 10, &parsed_val);

	if (parse_err || parsed_val <= 0 || parsed_val >= MAX_SEARCH_SIZE) {
		pr_err("search_size must be an integer in ragne (0, %d)\n",
		       MAX_SEARCH_SIZE);

		return parse_err || -EINVAL;
	}

	*(int *)kp->arg = parsed_val;

	return 0;
}

static const struct kernel_param_ops search_size_param_ops = {
	.set = search_size_param_set,
	.get = NULL,
};

module_param_cb(search_size, &search_size_param_ops, &search_size, 0);
MODULE_PARM_DESC(search_size, "the size to search for");

static int __init ex_bin_search_init(void)
{
	sort(modules, num_modules, sizeof(struct mod_size), mod_size_cmp, NULL);

	pr_info("modules:");
	for (int i = 0; i < num_modules; i++)
		pr_cont(" %s: %u,", modules[i].name, modules[i].size);
	pr_cont("\n");

	struct mod_size search_key = { .size = search_size };
	struct mod_size *found = bsearch(&search_key, modules, num_modules,
					 sizeof(struct mod_size), mod_size_cmp);
	if (found)
		pr_info("found module %s with size %u\n", found->name,
			found->size);
	else
		pr_info("no module with size %u\n", search_key.size);

	return 0;
}

static void __exit ex_bin_search_exit(void)
{
	pr_info("unloaded\n");
}

module_init(ex_bin_search_init);
module_exit(ex_bin_search_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION(
	"Module shows rbtree API usage with string values controlled by sysfs");
