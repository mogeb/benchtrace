#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#define CREATE_TRACE_POINTS
#include "empty_tp.h"

#define PROC_ENTRY_NAME "benchmod"
#define CPUS 8

#define BENCHMARK_MAGIC 'm'

#define IOCTL_BENCHMARK _IO(BENCHMARK_MAGIC, 0)
#define IOCTL_READ_RES  _IOR(BENCHMARK_MAGIC, 1, struct timspec*)

/*
 * 50 buckets, each bucket is a interval of 20 ns. With 50 buckets and a
 * granularity of 20ns, the histogram can cover values ranging between 0 and 1
 * microsecond.
 */
int nbuckets = 50;
int hist_granularity_ns = 20;
int **histogram;
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

int start_benchmark(unsigned int param)
{
    int i, ret = 0, loop = param;
    int this_cpu = smp_processor_id();
    struct timespec ts_start, ts_end, ts_diff;

    printk(KERN_INFO "Start_benchmark on CPU %d, param = %d\n",
           this_cpu, param);

    /*
     * Discard old values that haven't been read.
     */
    if(histogram[this_cpu]) {
        kfree(histogram[this_cpu]);
        histogram[this_cpu] = NULL;
    }
    /*
     * Allocate memory for the buckets of the histogram
     */
    histogram[this_cpu] = (int*) kmalloc(nbuckets * sizeof(int), __GFP_NOFAIL);
    for(i = 0; i < nbuckets; i++) {
        histogram[this_cpu][i] = 0;
    }

    if(!histogram[this_cpu]) {
        printk(KERN_INFO "Couldn't allocate buckets for CPU %d\n", this_cpu);
        ret = -1;
        goto done;
    }

    for(i = 0; i < loop; i++) {
        getnstimeofday(&ts_start);
        trace_empty_ioctl_4b(0);
        getnstimeofday(&ts_end);

        ts_diff = do_ts_diff(ts_start, ts_end);
        if(ts_diff.tv_nsec > nbuckets * hist_granularity_ns) {
            histogram[this_cpu][(nbuckets * hist_granularity_ns - 1) /
                    hist_granularity_ns]++;
        } else {
            histogram[this_cpu][ts_diff.tv_nsec / hist_granularity_ns]++;
        }
    }

done:
    if(this_cpu != smp_processor_id()) {
        printk("Ioctl migrated from %d to %d\n", this_cpu, smp_processor_id());
    }
    return ret;
}

long benchmod_ioctl(
        struct file *file,
        unsigned int ioctl_num,/* The number of the ioctl */
        unsigned long ioctl_param) /* The parameter to it */
{
    int ret = 0;

    switch(ioctl_num) {
    case IOCTL_BENCHMARK:
        start_benchmark(ioctl_param);
        break;

    default:
        printk("Invalid ioctl number\n");
    }

    return ret;
}

ssize_t benchmod_read(struct file *f, char *buf, size_t size, loff_t *offset)
{
    ssize_t ret = 0;
    int i = 0, j = 0, count = 0;
    int s, e;
    int res_hist[nbuckets];
    int print = 0;
    char *start;
    start = buf;

    printk("BENCHMOD_READ\n");

    if(!histogram) {
        ret = -1;
        goto done;
    }

    for(i = 0; i < nbuckets; i++) {
        res_hist[i] = 0;
    }

    for(i = 0; i < CPUS; i++) {
        if(histogram[i]) {
            print = 1;
            for(j = 0; j < nbuckets; j++) {
                res_hist[j] += histogram[i][j];
            }
            kfree(histogram[i]);
            histogram[i] = NULL;
        }
    }

    if(print) {
        for(i = 0; i < nbuckets; i++) {
            s = i * hist_granularity_ns;
            e = (i + 1) * hist_granularity_ns;
            count = sprintf(start + ret, "%d,%d,%d\n", s, e, res_hist[i]);
            ret += count;
        }
    }

done:
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
    sizes = (ssize_t*) kmalloc(CPUS * sizeof(ssize_t), GFP_KERNEL);
    histogram = (int**) kmalloc(CPUS * sizeof(int*), GFP_KERNEL);

    for(i = 0; i < CPUS; i++) {
        histogram[i] = NULL;
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

