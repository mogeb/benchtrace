#ifndef EXTRAE_BENCH_H
#define EXTRAE_BENCH_H

#include <popt.h>

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

#endif // EXTRAE_BENCH_H
