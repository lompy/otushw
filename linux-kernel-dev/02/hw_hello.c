#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/string.h>

#define MAX_MY_STR_LEN 1000

static int idx = 0;
static char ch_val = 0;
static int my_str_len = 0;
static char my_str[MAX_MY_STR_LEN + 1] = { 0 };

void update_my_str(void)
{
	if (ch_val == 0 || idx >= MAX_MY_STR_LEN || idx < 0) {
		pr_info("my_str not updated");
		return;
	}

	my_str[idx] = ch_val;
	pr_info("my_str updated");

	if (idx >= my_str_len) {
		int i;
		for (i = my_str_len; i < idx; ++i)
			my_str[i] = ' ';
		my_str_len = idx + 1;
		pr_info("my_str grew");
	}

	return;
}

static int idx_param_set(const char *val, const struct kernel_param *kp)
{
	int parsed_val;
	int parse_err;

	parse_err = kstrtoint(val, 10, &parsed_val);
	if (parse_err) {
		pr_err("invalid input: idx must be an integer in ragne [0, %d)\n",
		       MAX_MY_STR_LEN);
		return parse_err;
	}

	if (parsed_val < 0 || parsed_val >= MAX_MY_STR_LEN) {
		pr_err("invalid input: idx must be an integer in ragne [0, %d)\n",
		       MAX_MY_STR_LEN);
		return -EINVAL;
	}

	*(int *)kp->arg = parsed_val;

	return 0;
}

static int idx_param_get(char *buffer, const struct kernel_param *kp)
{
	return sprintf(buffer, "%d\n", *(int *)kp->arg);
}

static const struct kernel_param_ops idx_param_ops = {
	.set = idx_param_set,
	.get = idx_param_get,
};

static int single_char_param_set(const char *val, const struct kernel_param *kp)
{
	size_t len = strlen(val);

	if (len != 1 && !(len == 2 && val[len - 1] == '\n')) {
		pr_err("invalid input: ch_val must be single character\n");
		return -EINVAL;
	}

	if (*val == '\n') {
		pr_err("invalid input: ch_val must not be new line character\n");
		return -EINVAL;
	}

	*(char *)kp->arg = *val;

	update_my_str();

	return 0;
}

static int single_char_param_get(char *buffer, const struct kernel_param *kp)
{
	return sprintf(buffer, "%c\n", *(char *)kp->arg);
}

static const struct kernel_param_ops single_char_param_ops = {
	.set = single_char_param_set,
	.get = single_char_param_get,
};

module_param_cb(idx, &idx_param_ops, &idx,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(idx, "Char index in my_str string");
module_param_cb(ch_val, &single_char_param_ops, &ch_val,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(ch_val, "Char value for the 'idx' index");
module_param_string(my_str, my_str, sizeof(my_str), S_IRUSR | S_IRGRP);
MODULE_PARM_DESC(my_str, "A character string");

static int __init modinit(void)
{
	update_my_str();
	pr_info("init\n");

	return 0;
}

static void __exit modexit(void)
{
	pr_info("exit\n");
}

module_init(modinit);
module_exit(modexit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OldALoneFag");
MODULE_DESCRIPTION("A simple Hello Param module for the Linux kernel");
