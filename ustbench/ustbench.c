#define _GNU_SOURCE
#include <sched.h>
#include <sys/mman.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lightweight-ust.h"
#include "ustbench.h"
#include "utils.h"
#include "benchmod_utils.h"

/* Defines whether we track misses or branches */
#define TRACK_PMU_MISSES   1

#define TRACEPOINT_DEFINE
#include "tp.h"
#include <lttng/tracef.h>

struct perf_event_attr attr1, attr2, attr3, attr4;
struct measurement_cpu_perf *cpu_perf;
void (*do_tp)(size_t size);
static int nCpus;
char *open_filename;

char metric1_str[METRIC_LEN];
char metric2_str[METRIC_LEN];
char metric3_str[METRIC_LEN];
char metric4_str[METRIC_LEN];

static inline void do_lttng_ust_tp(size_t size)
{
    tracepoint(TRACEPOINT_PROVIDER, bench_tp_4b, 0);
}

static inline void do_tracef(size_t size)
{
    int i = 0;
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    tracef("__%lu %lu %d", ts.tv_sec, ts.tv_nsec, i);
}

static inline void do_lightweight_ust_tp(size_t size)
{
    int i = 0;

    trace_record_write(&i, sizeof(i));
}

static inline void do_printf_tp(size_t size)
{
    int i = 0;
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    fprintf(stdout, "__%lu %lu %d", ts.tv_sec, ts.tv_nsec, i);
}

static inline void do_none_tp(size_t size)
{
}

static inline void do_empty_syscall(size_t fd)
{
    ioctl(fd, IOCTL_EMPTY_CALL);
}

static inline void do_open(size_t fd)
{
    open(open_filename, O_RDONLY);
}

void perf_init()
{
    int i;

    for(i = 0; i < nCpus; i++) {
        cpu_perf[i].entries = (struct measurement_entry*) malloc(PER_CPU_ALLOC *
                    sizeof(struct measurement_entry));
        cpu_perf[i].pos = 0;
    }

    /**
      WARNING: LLC MISSES CRASHES!!!
    **/
    /*
    attr2.size = sizeof(struct perf_event_attr);
    attr2.pinned = 1;
    attr2.disabled = 0;
    attr2.type = PERF_TYPE_HW_CACHE;
    attr2.config = PERF_COUNT_HW_CACHE_LL | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    strncat(metric2, "LLC_misses", METRIC_LEN);
    */
    /**
      WARNING: LLC MISSES CRASHES!!!
    **/

#if TRACK_PMU_MISSES
    attr1.size = sizeof(struct perf_event_attr);
    attr1.pinned = 1;
    attr1.disabled = 0;
    attr1.type = PERF_TYPE_HW_CACHE;
    attr1.config = PERF_COUNT_HW_CACHE_L1D | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    attr1.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric1_str, "L1D_misses", METRIC_LEN);

    /* attr2 = cache misses */
    attr2.size = sizeof(struct perf_event_attr);
    attr2.pinned = 1;
    attr2.disabled = 0;
    attr2.type = PERF_TYPE_HARDWARE;
    attr2.config = PERF_COUNT_HW_CACHE_MISSES;
    attr2.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric2_str, "Cache_misses", METRIC_LEN);

    attr3.size = sizeof(struct perf_event_attr);
    attr3.pinned = 1;
    attr3.disabled = 0;
    attr3.type = PERF_TYPE_HARDWARE;
    attr3.config = PERF_COUNT_HW_CPU_CYCLES;
    attr3.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric3_str, "CPU_cycles", METRIC_LEN);

    attr4.size = sizeof(struct perf_event_attr);
    attr4.pinned = 1;
    attr4.disabled = 0;
    attr4.type = PERF_TYPE_HARDWARE;
    attr4.config = PERF_COUNT_HW_INSTRUCTIONS;
    attr4.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric4_str, "Instructions", METRIC_LEN);

#else // not TRACK_PMU_MISSES

    /* attr4 = dTLB-load-misses */
    attr1.size = sizeof(struct perf_event_attr);
    attr1.pinned = 1;
    attr1.disabled = 0;
    attr1.type = PERF_TYPE_HW_CACHE;
    attr1.config = PERF_COUNT_HW_CACHE_DTLB | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    attr1.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric1_str, "TLB_misses", METRIC_LEN);

    attr2.size = sizeof(struct perf_event_attr);
    attr2.pinned = 1;
    attr2.disabled = 0;
    attr2.type = PERF_TYPE_HARDWARE;
    attr2.config = PERF_COUNT_HW_BUS_CYCLES;
    attr2.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric2_str, "Bus_cycles", METRIC_LEN);

    attr3.size = sizeof(struct perf_event_attr);
    attr3.pinned = 1;
    attr3.disabled = 0;
    attr3.type = PERF_TYPE_HARDWARE;
    attr3.config = PERF_COUNT_HW_BRANCH_MISSES;
    attr3.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric3_str, "Branch_misses", METRIC_LEN);

    attr4.size = sizeof(struct perf_event_attr);
    attr4.pinned = 1;
    attr4.disabled = 0;
    attr4.type = PERF_TYPE_HARDWARE;
    attr4.config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
    attr4.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID;
    strncat(metric4_str, "Branch_instructions", METRIC_LEN);

#endif // TRACK_PMU_MISSES
}

static inline pid_t gettid()
{
    return syscall(SYS_gettid);
}

/* perf_event_open syscall wrapper */
static long
sys_perf_event_open(struct perf_event_attr *hw_event,
                    pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

struct perf_event_mmap_page *setup_perf(struct perf_event_attr *attr)
{
    void *perf_addr;
    int fd, ret;
    size_t pgsz;

    pgsz = sysconf(_SC_PAGESIZE);

    fd = sys_perf_event_open(attr, 0, -1, -1, 0);

    if (fd < 0)
        return NULL;

    perf_addr = mmap(NULL, pgsz, PROT_READ, MAP_SHARED, fd, 0);

    if (perf_addr == MAP_FAILED)
        return NULL;

    ret = close(fd);
    if (ret) {
        perror("Error closing perf memory mapping FD");
    }

    return perf_addr;
}

static unsigned long long rdpmc(unsigned int counter) {
  unsigned int low, high;

  __asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

  return (unsigned long long)low | ((unsigned long long)high) <<32;
}

#define barrier() __asm__ volatile("" ::: "memory")

/* Ingo's code for using rdpmc */
static inline unsigned long long mmap_read_self(void *addr) {

    struct perf_event_mmap_page *pc = addr;
    unsigned int seq,idx;

    unsigned long long count;

    do {
        seq = pc->lock;
        barrier();

        idx = pc->index;
        count = pc->offset;

        if (idx) {
            count += rdpmc(pc->index-1);
        }
        barrier();
    } while (pc->lock != seq);

    return count;
}

void *do_work(void *a)
{
    int i, min;
    struct worker_thread_args *args;
    args = (struct worker_thread_args*) a;
    int cpu = args->id;
    unsigned long pmu1_start, pmu1_end;
    unsigned long pmu2_start, pmu2_end;
    unsigned long pmu3_start, pmu3_end;
    unsigned long pmu4_start, pmu4_end;
    struct timespec ts_start, ts_end, ts_diff;
    struct perf_event_mmap_page *perf_mmap1, *perf_mmap2, *perf_mmap3,
            *perf_mmap4;
    int fd = 4; // for tracers other than syscall

    if(strcmp(popt_args.tracer, "syscall") == 0) {
        fd = open("/proc/benchmod", O_RDONLY);
        if(fd == -1) {
            printf("Error opening /proc/benchmod\n");
        }
    }

    perf_mmap1 = setup_perf(&attr1);
    if(!perf_mmap1) {
        printf("Couldn't allocate perf_mmap1\n");
    }
    perf_mmap2 = setup_perf(&attr2);
    if(!perf_mmap2) {
        printf("Couldn't allocate perf_mmap2\n");
    }
    perf_mmap3 = setup_perf(&attr3);
    if(!perf_mmap3) {
        printf("Couldn't allocate perf_mmap3\n");
    }
    perf_mmap4 = setup_perf(&attr4);
    if(!perf_mmap4) {
        printf("Couldn't allocate perf_mmap4\n");
    }

    printf("min = %d\n", min);
    for(i = 0; i < args->loops; i++) {
        int pos = cpu_perf[cpu].pos;
        pmu1_start = mmap_read_self(perf_mmap1);
        pmu2_start = mmap_read_self(perf_mmap2);
        pmu3_start = mmap_read_self(perf_mmap3);
        pmu4_start = mmap_read_self(perf_mmap4);

        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        do_tp(fd);
        clock_gettime(CLOCK_MONOTONIC, &ts_end);

        pmu4_end = mmap_read_self(perf_mmap4);
        pmu1_end = mmap_read_self(perf_mmap1);
        pmu2_end = mmap_read_self(perf_mmap2);
        pmu3_end = mmap_read_self(perf_mmap3);

        ts_diff = do_ts_diff(ts_start, ts_end);
        cpu_perf[cpu].entries[pos].pmu1 = pmu1_end - pmu1_start;
        cpu_perf[cpu].entries[pos].pmu2 = pmu2_end - pmu2_start;
        cpu_perf[cpu].entries[pos].pmu3 = pmu3_end - pmu3_start;
        cpu_perf[cpu].entries[pos].pmu4 = pmu4_end - pmu4_start;
        cpu_perf[cpu].entries[pos].latency = ts_diff.tv_sec * 1000000000 +
                ts_diff.tv_nsec;
        cpu_perf[cpu].pos++;
        cpu_perf[cpu].pos = cpu_perf[cpu].pos % PER_CPU_ALLOC;
    }

    if(strcmp(popt_args.tracer, "syscall") == 0) {
        close(fd);
    }

    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;
    struct worker_thread_args *worker_args;
    nCpus = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t *threads;
    poptContext pc;

    popt_args.tracer = "lttng-ust";
    popt_args.loops = 10;
    popt_args.nthreads = 1;

    parse_args(argc, argv, &pc);
    threads = (pthread_t*) malloc(popt_args.nthreads * sizeof(pthread_t));
    worker_args = (struct worker_thread_args*)
            malloc(popt_args.nthreads * sizeof(struct worker_thread_args));

    /* allocate cpu_perf before calling perf_init() */
    cpu_perf = malloc(nCpus * sizeof(struct measurement_cpu_perf));
    perf_init();

    if(strcmp(popt_args.tracer, "lw-ust") == 0) {
        printf("Setting tracer lw-ust\n");
        do_tp = do_lightweight_ust_tp;

    } else if (strcmp(popt_args.tracer, "printf") == 0) {
        printf("Setting tracer printf\n");

        fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        do_tp = do_printf_tp;
        close(fd);

    } else if (strcmp(popt_args.tracer, "none") == 0 ||
               strcmp(popt_args.tracer, "stap-ust") == 0) {
        printf("Setting tracer none\n");
        do_tp = do_none_tp;

    } else if(strcmp(popt_args.tracer, "syscall") == 0) {
        printf("Setting tracer syscall\n");
        do_tp = do_empty_syscall;

    } else if(strcmp(popt_args.tracer, "open") == 0) {
        printf("Setting tracer open\n");
        int len = popt_args.len;
        open_filename = malloc(len);
        strncpy(open_filename,
                "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", len);
        do_tp = do_open;

    } else if(strcmp(popt_args.tracer, "lttng-tracef") == 0) {
        printf("Setting tracer tracef\n");
        do_tp = do_tracef;

    } else {
        printf("Setting tracer lttng-ust\n");
        do_tp = do_lttng_ust_tp;
    }

    for(i = 0; i < popt_args.nthreads; i++) {
        worker_args[i].id = i;
        worker_args[i].loops = popt_args.loops;
        printf("nthreads = %d\n", popt_args.nthreads);
        /* Set CPU affinity for each thread*/
        cpu_set_t cpu_set;
        pthread_attr_t attr;
        CPU_ZERO(&cpu_set);
        CPU_SET(i % nCpus, &cpu_set);
        pthread_attr_init(&attr);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set);
        printf("loops = %d\n", worker_args->loops);

        pthread_create(&threads[i], &attr, do_work, (void*) &worker_args[i]);
    }

    for(i = 0; i < popt_args.nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    output_measurements();

    return 0;
}
