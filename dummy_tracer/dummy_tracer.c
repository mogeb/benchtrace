#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/vmalloc.h>
#include <linux/tracepoint.h>

#include <asm/tsc.h>

#include "dummy_tracer.h"
#include "measure.h"

#define PROC_ENTRY_NAME "dummy_tracer"
#define CPUS 4

#define BENCHMARK_MAGIC 'd'

#define IOCTL_BENCHMARK _IO(BENCHMARK_MAGIC, 0)
#define IOCTL_READ_RES  _IOR(BENCHMARK_MAGIC, 1, struct timspec*)
#define IOCTL_EMPTY_CALL _IO(BENCHMARK_MAGIC, 2)
#define BILLION 1000000000

ssize_t *sizes;

struct timespec do_ts_diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

struct timespec do_ts_add(struct timespec t1, struct timespec t2)
{
    long sec = t2.tv_sec + t1.tv_sec;
    long nsec = t2.tv_nsec + t1.tv_nsec;
    if (nsec >= BILLION) {
        nsec -= BILLION;
        sec++;
    }
    return (struct timespec){ .tv_sec = sec, .tv_nsec = nsec };
}

long benchmod_ioctl(
        struct file *file,
        unsigned int ioctl_num,/* The number of the ioctl */
        unsigned long ioctl_param) /* The parameter to it */
{
    int ret = 0;

    return ret;
}

ssize_t benchmod_read(struct file *f, char *buf, size_t size, loff_t *offset)
{
    ssize_t ret = 0;
    char *start;
    start = buf;

    printk("BENCHMOD_READ\n");
    printk("About to output measurements\n");
    output_measurements();

    return ret;
}

static int dummy_register_tracepoint(struct tp_module *tp_mod, void *func)
{
    int ret = 0, i;

    for (i = 0; i < tp_mod->mod->num_tracepoints; i++) {
        struct tracepoint *tp;

        tp = tp_mod->mod->tracepoints_ptrs[i];
        printk("[Dummy] Registering TP name: %s\n", tp->name);
        ret = tracepoint_probe_register(tp, func, NULL);
    }

    return ret;
}

static int dummy_unregister_tracepoint(struct tp_module *tp_mod, void *func)
{
    int ret = 0, i;

    for (i = 0; i < tp_mod->mod->num_tracepoints; i++) {
        struct tracepoint *tp;

        tp = tp_mod->mod->tracepoints_ptrs[i];
//        printk("[Dummy] Unregistering TP name: %s\n", tp->name);
        ret = tracepoint_probe_unregister(tp, func, NULL);
    }

    return ret;
}

void empty_probe(void)
{
    return;
}

static int dummy_tracepoint_notify(struct notifier_block *self,
        unsigned long val, void *data)
{
    struct tp_module *tp_mod = data;

    int ret = 0;

    switch (val) {
    case MODULE_STATE_COMING:
        ret = dummy_register_tracepoint(tp_mod, empty_probe);
        break;
    case MODULE_STATE_GOING:
        ret = dummy_unregister_tracepoint(tp_mod, empty_probe);
        break;
    default:
        break;
    }
    return ret;
}

static const struct file_operations empty_mod_operations = {
    .unlocked_ioctl = benchmod_ioctl,
    .read = benchmod_read
};

static struct notifier_block dummy_tracepoint_notifier = {
    .notifier_call = dummy_tracepoint_notify,
    .priority = 0,
};

/* Initialize the module - Register the character device */
static int __init dummy_init(void)
{
    int ret = 0;
    printk(KERN_INFO "Init benchmod\n");

    proc_create_data(PROC_ENTRY_NAME, S_IRUGO | S_IWUGO, NULL,
            &empty_mod_operations, NULL);

    printk("About to allocate measurements\n");
    alloc_measurements();

    printk("Registering dummy\n");
    register_tracepoint_module_notifier(&dummy_tracepoint_notifier);

    return ret;
}

static void __exit dummy_exit(void)
{
    printk(KERN_INFO "Exit benchmod\n");
    remove_proc_entry(PROC_ENTRY_NAME, NULL);
    printk("About to free measurements\n");
    free_measurements();
    printk("Unregistering dummy\n");
    unregister_tracepoint_module_notifier(&dummy_tracepoint_notifier);
}

module_init(dummy_init)
module_exit(dummy_exit)

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mohamad Gebai");
MODULE_DESCRIPTION("Tracing benchmark module");
