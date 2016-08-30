#ifndef USTBENCH_H
#define USTBENCH_H

#include <popt.h>

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
    char *tracer;
    int len;
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
    {
        "tracer", 't',
        POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
        &popt_args.tracer, 0, "tracer"
    },
    {
        "len", 'l',
        POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.len, 0, "len"
    },
    POPT_AUTOHELP
};

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

#endif // USTBENCH_H

