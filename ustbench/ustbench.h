#ifndef USTBENCH_H
#define USTBENCH_H

#include <popt.h>

struct popt_args {
    int nthreads;
    int loops;
    char *tracer;
    int len;
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

#endif // USTBENCH_H

