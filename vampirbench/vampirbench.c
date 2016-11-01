#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>

#include "vampirbench.h"
#include "libustperf.h"

#ifdef MANUAL
#include "vt_user.h"
#endif

static inline void do_vampirtrace_tp(size_t size)
{
#ifdef MANUAL
	VT_USER_START("work");
#endif

#ifdef MANUAL
	VT_USER_END("work");
#endif
    return;
}

void *do_work(void *a)
{
#if (defined(VTRACE))
    VT_ON();
#endif
    ustperf_do_work(do_vampirtrace_tp, a);
#if (defined(VTRACE))
    VT_OFF();
#endif
}

int main(int argc, char **argv)
{
//#if (defined(VTRACE))
//	VT_OFF();
//#endif
//#if (defined(MANUAL))
//        VT_USER_START("main");
//#endif
    int i, nCpus;
    pthread_t *threads;
    struct libustperf_args *worker_args;
    poptContext pc;

    popt_args.loops = 10;
    popt_args.nthreads = 1;

    nCpus = sysconf(_SC_NPROCESSORS_ONLN);
    parse_args(argc, argv, &pc);
    threads = (pthread_t*) malloc(popt_args.nthreads * sizeof(pthread_t));
    worker_args = (struct libustperf_args*)
            malloc(popt_args.nthreads * sizeof(struct libustperf_args));

    perf_init(nCpus);

    for (i = 0; i < popt_args.nthreads; i++) {
        worker_args[i].id = i;
        worker_args[i].loops = popt_args.loops;
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

    output_measurements(nCpus);
//#ifdef MANUAL
//        VT_USER_END("main");
//#endif

    return 0;
}
