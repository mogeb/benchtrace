#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>

#define CREATE_TRACE_POINTS
#include "empty_tp.h"

#define PROC_ENTRY_NAME "benchmod"

int empty_mod_ioctl(
		struct inode *inode,
		struct file *file,
		unsigned int ioctl_num,/* The number of the ioctl */
		unsigned long ioctl_param) /* The parameter to it */
{
	trace_empty_ioctl_1b(0);
	return 0;
}

static const struct file_operations empty_mod_operations = {
	.unlocked_ioctl = empty_mod_ioctl,
};

/* Initialize the module - Register the character device */
static int __init benchmod_init(void)
{
	int ret = 0;
	printk(KERN_INFO "Init benchmod\n");

	proc_create_data(PROC_ENTRY_NAME, S_IRUGO | S_IWUGO, NULL,
			&empty_mod_operations, NULL);

	return ret;
}

static void __exit benchmod_exit(void)
{
	printk(KERN_INFO "Exit benchmod\n");
	remove_proc_entry(PROC_ENTRY_NAME, NULL);
}

module_init(benchmod_init);
module_exit(benchmod_exit);

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mohamad Gebai");
MODULE_DESCRIPTION("Tracing benchmark module");

