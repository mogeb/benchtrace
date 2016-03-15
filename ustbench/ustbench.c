#define _GNU_SOURCE
#include <sched.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <sys/syscall.h>
#include <popt.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define METRIC_LEN 128
#define PER_CPU_ALLOC 100000
#define MIN(a, b) (a) < (b) ? (a) : (b)

#define TRACEPOINT_DEFINE
#include "tp.h"

struct perf_event_attr attr1, attr2, attr3, attr4;

struct measurement_entry {
    unsigned long pmu1;
    unsigned long pmu2;
    unsigned long pmu3;
    unsigned long pmu4;
    unsigned long latency;
};

struct measurement_cpu_perf {
    struct measurement_entry *entries;
    unsigned int pos;
};

struct popt_args {
    int nthreads;
    int loops;
};

struct worker_thread_args {
    int id;
    int loops;
};

struct popt_args popt_args;

struct poptOption options[] = {
    {
        NULL, 'n',
        POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.loops, 0,
        "Number of times to run the system call", "Desc"
    },
    {
        "threads", 'p',
        POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.nthreads, 0, "nthreads"
    },
    POPT_AUTOHELP
};

struct measurement_cpu_perf cpu_perf;

char metric1[METRIC_LEN];
char metric2[METRIC_LEN];
char metric3[METRIC_LEN];
char metric4[METRIC_LEN];

static void parse_args(int argc, char **argv, poptContext *pc)
{
    int val;

    *pc = poptGetContext(NULL, argc, (const char **)argv, options, 0);

    if (argc < 2) {
        poptPrintUsage(*pc, stderr, 0);
        return;
    }

    while ((val = poptGetNextOpt(*pc)) >= 0) {
        printf("poptGetNextOpt returned val %d\n", val);
    }
}

struct timespec do_ts_diff(struct timespec start,
                                         struct timespec end)
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

void perf_init()
{
    cpu_perf.entries = (struct measurement_entry*) malloc(PER_CPU_ALLOC *
                    sizeof(struct measurement_entry));
    cpu_perf.pos = 0;

    attr1.size = sizeof(struct perf_event_attr);
    attr1.pinned = 1;
    attr1.disabled = 0;
    attr1.type = PERF_TYPE_HW_CACHE;
    attr1.config = PERF_COUNT_HW_CACHE_L1D | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    strncat(metric1, "L1_misses", METRIC_LEN);

    attr2.size = sizeof(struct perf_event_attr);
    attr2.pinned = 1;
    attr2.disabled = 0;
    attr2.type = PERF_TYPE_HW_CACHE;
    attr2.config = PERF_COUNT_HW_CACHE_LL | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    strncat(metric2, "LLC_misses", METRIC_LEN);

    /* attr4 = dTLB-load-misses */
    attr3.size = sizeof(struct perf_event_attr);
    attr3.pinned = 1;
    attr3.disabled = 0;
    attr3.type = PERF_TYPE_HW_CACHE;
    attr3.config = PERF_COUNT_HW_CACHE_DTLB | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    strncat(metric3, "TLB_misses", METRIC_LEN);

    attr4.size = sizeof(struct perf_event_attr);
    attr4.pinned = 1;
    attr4.disabled = 0;
    attr4.type = PERF_TYPE_HARDWARE;
    attr4.config = PERF_COUNT_HW_INSTRUCTIONS;
    strncat(metric4, "Instructions", METRIC_LEN);
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

void *do_work(void *a)
{
    int i, min;
    struct worker_thread_args *args;
    args = (struct worker_thread_args*) a;
    printf("loops = %d\n", args->loops);
    unsigned long pmu1_start, pmu1_end;
    unsigned long pmu2_start, pmu2_end;
    unsigned long pmu3_start, pmu3_end;
    unsigned long pmu4_start, pmu4_end;
    pid_t tid = gettid();
    struct timespec ts_start, ts_end, ts_diff;
    FILE *outfile;

    int fdPmu1 = sys_perf_event_open(&attr1, tid, -1, -1, 0);
    int fdPmu2 = sys_perf_event_open(&attr2, tid, -1, -1, 0);
    int fdPmu3 = sys_perf_event_open(&attr3, tid, -1, -1, 0);
    int fdPmu4 = sys_perf_event_open(&attr4, tid, -1, -1, 0);

    min = MIN(args->loops, PER_CPU_ALLOC);
    printf("min = %d\n", min);
    for(i = 0; i < min; i++) {
        read(fdPmu1, &pmu1_start, sizeof(pmu1_start));
        read(fdPmu2, &pmu2_start, sizeof(pmu2_start));
        read(fdPmu3, &pmu3_start, sizeof(pmu3_start));
        read(fdPmu4, &pmu4_start, sizeof(pmu4_start));
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        tracepoint(TRACEPOINT_PROVIDER, bench_tp_4b, 0);
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        /* read instructions PMU first to have the smallest number possible */
        read(fdPmu4, &pmu4_end, sizeof(pmu4_end));
        read(fdPmu1, &pmu1_end, sizeof(pmu1_end));
        read(fdPmu2, &pmu2_end, sizeof(pmu2_end));
        read(fdPmu3, &pmu3_end, sizeof(pmu3_end));

        ts_diff = do_ts_diff(ts_start, ts_end);
        cpu_perf.entries[i].pmu1 = pmu1_end - pmu1_start;
        cpu_perf.entries[i].pmu2 = pmu2_end - pmu2_start;
        cpu_perf.entries[i].pmu3 = pmu3_end - pmu3_start;
        cpu_perf.entries[i].pmu4 = pmu4_end - pmu4_start;
        cpu_perf.entries[i].latency = ts_diff.tv_sec * 1000000000 +
                ts_diff.tv_nsec;
    }

    outfile = fopen("/tmp/out.csv", "w+");
    printf("latency,%s,%s,%s,%s\n", metric1, metric2, metric3, metric4);
    fprintf(outfile, "latency,%s,%s,%s,%s\n", metric1, metric2, metric3, metric4);

    for(i = 0; i < min; i++) {
        fprintf(outfile, "%lu,%lu,%lu,%lu,%lu\n", cpu_perf.entries[i].latency,
                cpu_perf.entries[i].pmu1,
                cpu_perf.entries[i].pmu2,
                cpu_perf.entries[i].pmu3,
                cpu_perf.entries[i].pmu4);
//        printf("%lu,%lu,%lu,%lu,%lu\n", cpu_perf.entries[i].latency,
//                cpu_perf.entries[i].pmu1,
//                cpu_perf.entries[i].pmu2,
//                cpu_perf.entries[i].pmu3,
//                cpu_perf.entries[i].pmu4);
    }

    fclose(outfile);

    return 0;
}

int main(int argc, char **argv)
{
    int i;
    struct worker_thread_args *worker_args;
    int ncpus = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t *threads;
    poptContext pc;

    parse_args(argc, argv, &pc);
    threads = (pthread_t*) malloc(popt_args.nthreads * sizeof(pthread_t));
    worker_args = (struct worker_thread_args*)
            malloc(sizeof(struct worker_thread_args));

    perf_init();

    worker_args->id = i;
    worker_args->loops = popt_args.loops;
    for(i = 0; i < popt_args.nthreads; i++) {
        printf("nthreads = %d\n", popt_args.nthreads);
        /* Set CPU affinity for each thread*/
        cpu_set_t cpu_set;
        pthread_attr_t attr;
        CPU_ZERO(&cpu_set);
        CPU_SET(i % ncpus, &cpu_set);
        pthread_attr_init(&attr);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set);
        printf("loops = %d\n", worker_args->loops);

        pthread_create(&threads[i], &attr, do_work, (void*) worker_args);
    }

    for(i = 0; i < popt_args.nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
