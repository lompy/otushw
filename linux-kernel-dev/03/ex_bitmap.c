#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/bitmap.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

#define MAX_NBITS 1000

static int nbits = 100;

unsigned long *bitmap;

static int nbits_param_set(const char *val, const struct kernel_param *kp)
{
	int parsed_val;
	int parse_err = kstrtoint(val, 10, &parsed_val);

	if (parse_err || parsed_val <= 0 || parsed_val >= MAX_NBITS) {
		pr_err("nbits must be an integer in ragne (0, %d)\n",
		       MAX_NBITS);

		return parse_err || -EINVAL;
	}

	*(int *)kp->arg = parsed_val;

	return 0;
}

static const struct kernel_param_ops nbits_param_ops = {
	.set = nbits_param_set,
	.get = NULL,
};

module_param_cb(nbits, &nbits_param_ops, &nbits, 0);
MODULE_PARM_DESC(nbits, "bitmap nbits meaning number of nbits");

static __init int init_ex_bitmap(void)
{
	bitmap = bitmap_alloc(nbits, GFP_KERNEL);
	if (!bitmap) {
		pr_err("bitmap_alloc: no memory\n");

		return -ENOMEM;
	}
	int longs = BITS_TO_LONGS(nbits);
	bitmap_zero(bitmap, nbits);
	pr_info("allocated bitmap of %d bits (%d longs): %*pb\n", nbits, longs,
		nbits, bitmap);

	for (int i = 1; i < nbits; i += 2) {
		bitmap_set(bitmap, i, 1);
	}
	pr_info("set odd nbits: %*pb\n", nbits, bitmap);

	bitmap_zero(bitmap, nbits);
	pr_info("zero: %*pb\n", nbits, bitmap);

	return 0;
}

static __exit void exit_ex_bitmap(void)
{
	if (bitmap)
		kfree(bitmap);

	pr_info("unloaded\n");
}

module_init(init_ex_bitmap);
module_exit(exit_ex_bitmap);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roman Charushin");
MODULE_DESCRIPTION(
	"Module shows bitmap API usage for a bitmap of NBITS controlled by param");
