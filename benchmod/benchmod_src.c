#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/module.h>	/* Specifically, a module */
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/vmalloc.h>

#include <asm/tsc.h>

#include "benchmod.h"
#include "measure.h"

#define CREATE_TRACE_POINTS
#include "empty_tp.h"

#define PROC_ENTRY_NAME "benchmod"
#define CPUS 8

#define BENCHMARK_MAGIC 'm'

#define IOCTL_BENCHMARK _IO(BENCHMARK_MAGIC, 0)
#define IOCTL_READ_RES  _IOR(BENCHMARK_MAGIC, 1, struct timspec*)
#define IOCTL_EMPTY_CALL _IO(BENCHMARK_MAGIC, 2)
#define BILLION 1000000000

/*
 * 50 buckets, each bucket is a interval of 20 ns. With 50 buckets and a
 * granularity of 20ns, the histogram can cover values ranging between 0 and 1
 * microsecond.
 */
int hist_granularity_ns = 5;
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

void tp_512b(void)
{
    trace_empty_ioctl_512b(zero_256b, zero_256b);
}

void tp_768b(void)
{
    trace_empty_ioctl_768b(zero_256b, zero_256b, zero_256b);
}

void tp_1kb(void)
{
    trace_empty_ioctl_1kb(zero_256b, zero_256b, zero_256b, zero_256b);
}

void tp_1p5kb(void)
{
    trace_empty_ioctl_1p5kb(zero_256b, zero_256b, zero_256b, zero_256b,
                            zero_256b, zero_256b);
}

void tp_2kb(void)
{
    trace_empty_ioctl_2kb(zero_256b, zero_256b, zero_256b, zero_256b,
                          zero_256b, zero_256b, zero_256b, zero_256b);
}

int start_benchmark(struct benchmod_arg arg)
{
    int i, ret = 0, loop = arg.loop;
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

    case 512:
        do_tp = tp_512b;
        break;

    case 768:
        do_tp = tp_768b;
        break;

    case 1024:
        do_tp = tp_1kb;
        break;

    case 1536:
        do_tp = tp_1p5kb;
        break;

    case 2048:
        do_tp = tp_2kb;
        break;

    default:
        do_tp = tp_4b;
    }

    /*
     * Discard old values that haven't been read.
     */
    for_each_online_cpu(cpu) {

    }

    BENCH_PREAMBULE;
    for(i = 0; i < loop; i++) {
        BENCH_GET_TS1;
        do_tp();
        BENCH_GET_TS2;
    }
    BENCH_APPEND;
    print = 1;

    return ret;
}

long benchmod_ioctl(
        struct file *file,
        unsigned int ioctl_num,/* The number of the ioctl */
        unsigned long ioctl_param) /* The parameter to it */
{
    if(ioctl_num == IOCTL_EMPTY_CALL) {
        return;
    }
    int ret = 0;
    struct benchmod_arg *benchmod_arg;
    benchmod_arg = (struct benchmod_arg*) ioctl_param;

    if(!ioctl_param) {
        printk("ioctl_param is NULL\n");
        return -1;
    }

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
    char *start;
    start = buf;

    printk("BENCHMOD_READ\n");
    printk("About to output measurements\n");
    output_measurements();

    return ret;
}

static const struct file_operations empty_mod_operations = {
    .unlocked_ioctl = benchmod_ioctl,
    .read = benchmod_read
};


/* Initialize the module - Register the character device */
static int __init benchmod_init(void)
{
    int ret = 0;
    printk(KERN_INFO "Init benchmod\n");

    proc_create_data(PROC_ENTRY_NAME, S_IRUGO | S_IWUGO, NULL,
            &empty_mod_operations, NULL);

    printk("About to allocate measurements\n");
    alloc_measurements();

    return ret;
}

static void __exit benchmod_exit(void)
{
    printk(KERN_INFO "Exit benchmod\n");
    remove_proc_entry(PROC_ENTRY_NAME, NULL);
    printk("About to free measurements\n");
    free_measurements();
}

module_init(benchmod_init)
module_exit(benchmod_exit)

MODULE_LICENSE("GPL and additional rights");
MODULE_AUTHOR("Mohamad Gebai");
MODULE_DESCRIPTION("Tracing benchmark module");
