#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>
#include <linux/slab.h>

#define CREATE_TRACE_POINTS
#include "empty_tp.h"

#define PROC_ENTRY_NAME "benchmod"
#define CPUS 8

struct timespec **calltimes;

long benchmod_ioctl(
        struct file *file,
        unsigned int ioctl_num,/* The number of the ioctl */
        unsigned long ioctl_param) /* The parameter to it */
{
    int i, ret = 0, loop = ioctl_param;
    int this_cpu = smp_processor_id();
    struct timespec *iterator_ts;

    /*
     * Discard old values that haven't been read.
     */
    if(calltimes[this_cpu]) {
        kfree(calltimes[this_cpu]);
        calltimes[this_cpu] = NULL;
    }
    /*
     * Allocate memory for all the calls that will be done, that is 'loop'
     * number of calls.
     */
    calltimes[this_cpu] = (struct timespec*)
            kmalloc(loop * sizeof(struct timespec), GFP_KERNEL);

    if(!calltimes[this_cpu]) {
        printk(KERN_INFO "Couldn't allocate %d spots\n", loop);
        ret = -1;
        goto done;
    }
    iterator_ts = calltimes[this_cpu];

    for(i = 0; i < loop; i++) {
        getnstimeofday(iterator_ts++);
        trace_empty_ioctl_1b(0);
    }

    printk(KERN_INFO "Did %d loops\n", loop);

done:
    return ret;
}

ssize_t benchmod_read(struct file *f, char *buf, size_t size, loff_t *offset)
{
    ssize_t ret = -1;

    return ret;
}

static const struct file_operations empty_mod_operations = {
    .unlocked_ioctl = benchmod_ioctl,
    .read = benchmod_read
};


/* Initialize the module - Register the character device */
static int __init benchmod_init(void)
{
    int ret = 0, i = 0;
    printk(KERN_INFO "Init benchmod\n");

    /*
     * Allocate some memory to store the latency values
     */
    calltimes = (struct timespec**)
            kmalloc(CPUS * sizeof(struct timespec*), GFP_KERNEL);

    for(i = 0; i < CPUS; i++) {
        calltimes[i] = NULL;
    }

    proc_create_data(PROC_ENTRY_NAME, S_IRUGO | S_IWUGO, NULL,
            &empty_mod_operations, NULL);

    return ret;
}

static void __exit benchmod_exit(void)
{
    printk(KERN_INFO "Exit benchmod\n");
    remove_proc_entry(PROC_ENTRY_NAME, NULL);
}

module_init(benchmod_init)
module_exit(benchmod_exit)

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mohamad Gebai");
MODULE_DESCRIPTION("Tracing benchmark module");

