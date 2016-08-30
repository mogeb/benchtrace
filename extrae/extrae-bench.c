/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                   Extrae                                  *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#include "utils.h"
#include "ustbench.h"
#include "extrae_user_events.h"
#include "libustperf.h"

void do_extrae_tp(size_t size)
{
    Extrae_event(1000, size);
//    Extrae_counters();
//    Extrae_user_function(1);
}

void *do_work(void *a)
{
    ustperf_do_work(do_extrae_tp, a);
}

//void *routine1 (void *a)
//{
//    int i;
//    unsigned long pmu1_start, pmu1_end;
//    unsigned long pmu2_start, pmu2_end;
//    unsigned long pmu3_start, pmu3_end;
//    unsigned long pmu4_start, pmu4_end;

//    struct timespec ts_start, ts_end, ts_diff;
//    struct perf_event_mmap_page *perf_mmap1, *perf_mmap2, *perf_mmap3,
//            *perf_mmap4;
//    struct worker_thread_args *args;
//    args = (struct worker_thread_args*) a;
//    int cpu = args->id;
//    int tid = gettid();

//    perf_mmap1 = setup_perf(&attr1);
//    if(!perf_mmap1) {
//        printf("Couldn't allocate perf_mmap1\n");
//    }
//    perf_mmap2 = setup_perf(&attr2);
//    if(!perf_mmap2) {
//        printf("Couldn't allocate perf_mmap2\n");
//    }
//    perf_mmap3 = setup_perf(&attr3);
//    if(!perf_mmap3) {
//        printf("Couldn't allocate perf_mmap3\n");
//    }
//    perf_mmap4 = setup_perf(&attr4);
//    if(!perf_mmap4) {
//        printf("Couldn't allocate perf_mmap4\n");
//    }

//    for(i = 0; i < 24999; i++) {
//        int pos = cpu_perf[cpu].pos;
//        pmu1_start = mmap_read_self(perf_mmap1);
//        pmu2_start = mmap_read_self(perf_mmap2);
//        pmu3_start = mmap_read_self(perf_mmap3);
//        pmu4_start = mmap_read_self(perf_mmap4);

//        clock_gettime(CLOCK_MONOTONIC, &ts_start);
//        /* Extrae_user_function(1); */
//        Extrae_event(1000, i + 1); // 1000 just because and make loop start at 1
//        /* Extrae_counters(); */
//        clock_gettime(CLOCK_MONOTONIC, &ts_end);

//        pmu4_end = mmap_read_self(perf_mmap4);
//        pmu1_end = mmap_read_self(perf_mmap1);
//        pmu2_end = mmap_read_self(perf_mmap2);
//        pmu3_end = mmap_read_self(perf_mmap3);

//        ts_diff = do_ts_diff(ts_start, ts_end);
//        cpu_perf[cpu].entries[pos].pmu1 = pmu1_end - pmu1_start;
//        cpu_perf[cpu].entries[pos].pmu2 = pmu2_end - pmu2_start;
//        cpu_perf[cpu].entries[pos].pmu3 = pmu3_end - pmu3_start;
//        cpu_perf[cpu].entries[pos].pmu4 = pmu4_end - pmu4_start;
//        cpu_perf[cpu].entries[pos].latency = ts_diff.tv_sec * 1000000000 +
//                ts_diff.tv_nsec;
//        cpu_perf[cpu].pos++;
//        cpu_perf[cpu].pos = cpu_perf[cpu].pos % PER_CPU_ALLOC;
//    }
//}

int main (int argc, char *argv[])
{
    int i, nCpus;
    pthread_t *threads;
    struct worker_thread_args *worker_args;
    poptContext pc;

    nCpus = sysconf(_SC_NPROCESSORS_ONLN);
    parse_args(argc, argv, &pc);
    threads = (pthread_t*) malloc(popt_args.nthreads * sizeof(pthread_t));
    worker_args = (struct worker_thread_args*)
            malloc(popt_args.nthreads * sizeof(struct worker_thread_args));

    perf_init(nCpus);
//    Extrae_init ();

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
    for (i = 0; i < popt_args.nthreads; i++)
        pthread_join(threads[i], NULL);

    output_measurements(nCpus);
    Extrae_fini();

    return 0;
}
