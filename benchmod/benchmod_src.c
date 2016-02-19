#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/vmalloc.h>

#include <asm/tsc.h>

#include "benchmod.h"

#define CREATE_TRACE_POINTS
#include "empty_tp.h"

#define PROC_ENTRY_NAME "benchmod"
#define CPUS 8

#define BENCHMARK_MAGIC 'm'

#define IOCTL_BENCHMARK _IO(BENCHMARK_MAGIC, 0)
#define IOCTL_READ_RES  _IOR(BENCHMARK_MAGIC, 1, struct timspec*)
#define NBUCKETS 100

#define irq_stats(x)            (&per_cpu(irq_stat, x))

#define BENCH_PREAMBULE unsigned long _bench_flags; \
    unsigned int _bench_nmi; \
    int *_bench_h; \
    unsigned long *_bench_a; \
    u64 _bench_ts1 = 0, _bench_ts2 = 0, _bench_diff; \
    _bench_a = this_cpu_ptr(&averages); \
    _bench_h = this_cpu_ptr(histos); \
    local_irq_save(_bench_flags); \
    _bench_nmi = irq_stats(smp_processor_id())->__nmi_count

#define BENCH_GET_TS1 _bench_ts1 = get_cycles();

#define BENCH_GET_TS2 _bench_ts2 = get_cycles();

#define BENCH_APPEND if (_bench_nmi == irq_stats(smp_processor_id())->__nmi_count) { \
        _bench_diff = _bench_ts2 - _bench_ts1; \
        if(_bench_diff > NBUCKETS * hist_granularity_ns) { \
            _bench_h[(NBUCKETS * hist_granularity_ns - 1) / hist_granularity_ns]++; \
        } else { \
            _bench_h[_bench_diff / hist_granularity_ns]++; \
        } \
        *_bench_a += (unsigned long)_bench_diff; \
        printk("_bench_a = %ld, bench_diff = %ld\n", *_bench_a, (unsigned long)_bench_diff); \
    } \
    local_irq_restore(_bench_flags)

/*
 * 50 buckets, each bucket is a interval of 20 ns. With 50 buckets and a
 * granularity of 20ns, the histogram can cover values ranging between 0 and 1
 * microsecond.
 */
int hist_granularity_ns = 10;
ssize_t *sizes;
char zero_8b[8] = { 0 };
char zero_16b[16] = { 0 };
char zero_32b[32] = { 0 };
char zero_64b[64] = { 0 };
char zero_128b[128] = { 0 };
char zero_192b[192] = { 0 };
char zero_256b[256] = { 0 };
void (*do_tp)(void);
int print = 0;

DEFINE_PER_CPU(int[NBUCKETS], histos);
DEFINE_PER_CPU(unsigned long, averages);

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
    int *h;
    unsigned long *a;
    int cpu;

    printk(KERN_INFO "Start_benchmark on CPU %d, loop = %d, tp_size = %d\n",
           smp_processor_id(), arg.loop, arg.tp_size);

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
    for_each_online_cpu(cpu) {
        a = per_cpu_ptr(&averages, cpu);
        h = per_cpu_ptr(histos, cpu);
        for(i = 0; i < NBUCKETS; i++) {
            h[i] = 0;
        }
        if(a) {
            *a = 0;
        }
        else {
            ret = -1;
            goto done;
        }
    }

    for(i = 0; i < loop; i++) {
        BENCH_PREAMBULE;
        BENCH_GET_TS1;
        do_tp();
        BENCH_GET_TS2;
        BENCH_APPEND;
    }
    print = 1;

done:
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
    int i = 0, count = 0;
    int s, e;
    int res_hist[NBUCKETS] = { 0 }, loop = 0;
    unsigned long average = 0;
    int *h, cpu = smp_processor_id();
    unsigned long *a;
    char *start;
    start = buf;

    printk("BENCHMOD_READ\n");

    for_each_online_cpu(cpu) {
        a = per_cpu_ptr(&averages, cpu);
        h = per_cpu_ptr(histos, cpu);
        for(i = 0; i < NBUCKETS; i++) {
            res_hist[i] += h[i];
        }
        average += *a;
    }

    if(print) {
        for(i = 0; i < NBUCKETS; i++) {
            s = i * hist_granularity_ns;
            e = (i + 1) * hist_granularity_ns;
            count = sprintf(start + ret, "%d,%d,%d\n", s, e, res_hist[i]);
            ret += count;
            loop += res_hist[i];
        }
        average /= loop;
        count = sprintf(start + ret, "%ld\n", average);
        ret += count;
    }
    print = 0;

    return ret;
}

static const struct file_operations empty_mod_operations = {
    .unlocked_ioctl = benchmod_ioctl,
    .read = benchmod_read
};


/* Initialize the module - Register the character device */
static int __init benchmod_init(void)
{
    int ret = 0, cpu;
    int *h;
    unsigned long *a;
    printk(KERN_INFO "Init benchmod\n");

    for_each_online_cpu(cpu) {
        printk("CPU %d is allocating\n", cpu);
        h = per_cpu_ptr(histos, cpu);
        a = per_cpu_ptr(&averages, cpu);
        h = (int*) vmalloc(NBUCKETS * sizeof(int));
        a = (unsigned long*) vmalloc(sizeof(unsigned long));
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
