#ifndef LIBUST_PERF_H
#define LIBUST_PERF_H

#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <popt.h>
#include "utils.h"

#define TRACK_PMU_MISSES  1

struct libustperf_args {
    int id;
    int loops;
};

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

struct perf_event_attr *attr1, *attr2, *attr3, *attr4;
struct measurement_cpu_perf *cpu_perf;

char metric1_str[METRIC_LEN];
char metric2_str[METRIC_LEN];
char metric3_str[METRIC_LEN];
char metric4_str[METRIC_LEN];

void output_measurements(int nCpus);
void perf_init(int nCpus);

static unsigned long long rdpmc(unsigned int counter) {
  unsigned int low, high;

  __asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

  return (unsigned long long)low | ((unsigned long long)high) <<32;
}

#define barrier() __asm__ volatile("" ::: "memory")

/* Ingo's code for using rdpmc */
static inline unsigned long long mmap_read_self(struct perf_event_mmap_page *addr) {

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

void ustperf_do_work(void (*func)(), void *a);

#endif // LIBUST_PERF_H
