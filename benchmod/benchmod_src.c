#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#include "benchmod.h"

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
int **histograms;
unsigned long *averages;
ssize_t *sizes;
char zero_8b[8] = { 0 };
char zero_16b[16] = { 0 };
char zero_32b[32] = { 0 };
char zero_64b[64] = { 0 };
char zero_128b[128] = { 0 };
char zero_192b[192] = { 0 };
char zero_256b[256] = { 0 };
void (*do_tp)(void);

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

void tp_4b(void)
{
    trace_empty_ioctl_4b(0);
}

void tp_8b(void)
{
    trace_empty_ioctl_8b(zero_8b);
}

void tp_16b(void)
{
    trace_empty_ioctl_16b(zero_16b);
}

void tp_32b(void)
{
    trace_empty_ioctl_32b(zero_32b);
}

void tp_64b(void)
{
    trace_empty_ioctl_64b(zero_64b);
}

void tp_128b(void)
{
    trace_empty_ioctl_128b(zero_128b);
}

void tp_192b(void)
{
    trace_empty_ioctl_192b(zero_192b);
}

void tp_256b(void)
{
    trace_empty_ioctl_256b(zero_256b);
}

int start_benchmark(struct benchmod_arg arg)
{
    int i, ret = 0, loop = arg.loop;
    int this_cpu = smp_processor_id();
    struct timespec ts_start, ts_end, ts_diff;

    printk(KERN_INFO "Start_benchmark on CPU %d, loop = %d, tp_size = %d\n",
           this_cpu, arg.loop, arg.tp_size);

    /*
     * Update tracepoint call
     */
    switch(arg.tp_size) {
    case 8:
        do_tp = tp_8b;
        break;

    case 16:
        do_tp = tp_16b;
        break;

    case 32:
        do_tp = tp_32b;
        break;

    case 64:
        do_tp = tp_64b;
        break;

    case 128:
        do_tp = tp_128b;
        break;

    case 192:
        do_tp = tp_192b;
        break;

    case 256:
        do_tp = tp_256b;
        break;

    default:
        do_tp = tp_4b;
    }

    /*
     * Discard old values that haven't been read.
     */
    if(histograms[this_cpu]) {
        kfree(histograms[this_cpu]);
        histograms[this_cpu] = NULL;
    }
    for(i = 0; i < CPUS; i++) {
        averages[i] = 0;
    }

    /*
     * Allocate memory for the buckets of the histogram
     */
    histograms[this_cpu] = (int*) kmalloc(nbuckets * sizeof(int), __GFP_NOFAIL);
    for(i = 0; i < nbuckets; i++) {
        histograms[this_cpu][i] = 0;
    }

    if(!histograms[this_cpu]) {
        printk(KERN_INFO "Couldn't allocate buckets for CPU %d\n", this_cpu);
        ret = -1;
        goto done;
    }

    for(i = 0; i < loop; i++) {
        getnstimeofday(&ts_start);
        do_tp();
        getnstimeofday(&ts_end);

        ts_diff = do_ts_diff(ts_start, ts_end);
        if(ts_diff.tv_nsec > nbuckets * hist_granularity_ns) {
            histograms[this_cpu][(nbuckets * hist_granularity_ns - 1) /
                    hist_granularity_ns]++;
        } else {
            histograms[this_cpu][ts_diff.tv_nsec / hist_granularity_ns]++;
        }
        averages[this_cpu] += ts_diff.tv_nsec;
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
    struct benchmod_arg *benchmod_arg;
    benchmod_arg = (struct benchmod_arg*) ioctl_param;

    printk("BENCH ARGS RECEIVED:\n");
    printk("loops: %d, tp_size: %d\n", benchmod_arg->loop,
           benchmod_arg->tp_size);

    switch(ioctl_num) {
    case IOCTL_BENCHMARK:
        start_benchmark(*benchmod_arg);
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
    int res_hist[nbuckets], loop = 0;
    unsigned long average = 0;
    int print = 0;
    char *start;
    start = buf;

    printk("BENCHMOD_READ\n");

    if(!histograms) {
        ret = -1;
        goto done;
    }

    for(i = 0; i < nbuckets; i++) {
        res_hist[i] = 0;
    }

    for(i = 0; i < CPUS; i++) {
        average += averages[i];
        printk("Average on CPU %d = %ld\n", i, averages[i]);
        if(histograms[i]) {
            print = 1;
            for(j = 0; j < nbuckets; j++) {
                res_hist[j] += histograms[i][j];
            }
            kfree(histograms[i]);
            histograms[i] = NULL;
        }
    }

    if(print) {
        for(i = 0; i < nbuckets; i++) {
            s = i * hist_granularity_ns;
            e = (i + 1) * hist_granularity_ns;
            count = sprintf(start + ret, "%d,%d,%d\n", s, e, res_hist[i]);
            ret += count;
            loop += res_hist[i];
        }

        average /= loop;
        count = sprintf(start + ret, "%ld\n", average);
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
    histograms = (int**) kmalloc(CPUS * sizeof(int*), GFP_KERNEL);
    averages = (unsigned long*) kmalloc(CPUS * sizeof(unsigned long),
                                        GFP_KERNEL);

    for(i = 0; i < CPUS; i++) {
        histograms[i] = NULL;
        averages[i] = 0;
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

