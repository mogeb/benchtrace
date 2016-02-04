#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <popt.h>
#include <pthread.h>
#include <time.h>

#define BENCHMOD_NAME "/proc/benchmod"

struct popt_args {
	int clear;
	int loop;
	int nthreads;
	char *outfile;
	char *tracer;
    int dump;
};

struct popt_args popt_args;
pthread_barrier_t barrier;

struct poptOption options[] = {
	{
        NULL, 'n',
		POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.loop, 0,
        "Number of times to run the system call", "Desc"
	},
	{
        "output", 'o',
		POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
        &popt_args.outfile,  0,
        "File to append to", "Desc file"
	},
	{
        "tracer", 't',
		POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
        &popt_args.tracer, 0,
        "Tracer", "tracer"
	},
	{
        "threads", 'p',
		POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.nthreads,  0,
        "Number of threads", "Nthreads"
	},
	{
        "clear", 'c',
		POPT_ARG_NONE | POPT_ARGFLAG_OPTIONAL,
        &popt_args.clear, 0,
        "Clear file", "clear"
    },
    {
        "dump", 'd',
        POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.clear, 0,
        "Dump all values", "dump all values"
    },
	POPT_AUTOHELP
};


struct thread_args {
    int id;
	int ntimes;
    struct timespec *calltimes;
};

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

static void dump(char *outfile, struct timespec **calltimes)
{
    FILE *file, *heatmap_file;
    int i, j;
    int loop_per_thread = popt_args.loop / popt_args.nthreads;
    unsigned long average = 0;
    unsigned long totaltime = 0;
    struct timespec diff;

	file = fopen(outfile, popt_args.clear ? "w+" : "a+");
    if(popt_args.dump) {
        heatmap_file = fopen("heatmap.out", "w+");
    }

	if(popt_args.clear) {
        fprintf(file, "tracer,total call time,average call time,number of events,number of threads\n");
	}

    for(i = 0; i < popt_args.nthreads; i++) {
        for(j = 1; j < loop_per_thread; j++) {
            diff = do_ts_diff(calltimes[i][j - 1], calltimes[i][j]);
            totaltime += (diff.tv_sec * 1000000000 + diff.tv_nsec);

            if(popt_args.dump) {
                fprintf(heatmap_file, "%ld,%ld\n",
                    calltimes[i][j].tv_sec * 1000000000 + calltimes[i][j].tv_nsec,
                    diff.tv_sec * 1000000000 + diff.tv_nsec);
            }
        }
    }

    average = totaltime / popt_args.loop;
    fprintf(file, "%s,%ld,%ld,%d,%d\n", popt_args.tracer, totaltime, average, popt_args.loop,
        popt_args.nthreads);

    fclose(file);
}

void *do_work(void *args)
{
    struct thread_args *arg = args;
    struct timespec *start;
    int uid;

    start = arg->calltimes;
    pthread_barrier_wait(&barrier);
    while(arg->calltimes < start + arg->ntimes) {
        clock_gettime(CLOCK_MONOTONIC, arg->calltimes++);
        uid = getuid();
    }
}

int main(int argc, char **argv)
{
	popt_args.outfile = "default";
	popt_args.clear = 0;
	popt_args.tracer = "none";
    popt_args.nthreads = 2;
    popt_args.loop = 100;
    popt_args.dump = 0;
    struct timespec **calltimes;
    int nthreads, loops_per_thread;
    int i, ret = 0;
    struct thread_args *pthread_args;
    poptContext pc;
	pthread_t *threads;

	parse_args(argc, argv, &pc);
	printf("nthreads = %d, loop = %d\n", popt_args.nthreads, popt_args.loop);

    nthreads = popt_args.nthreads;
    loops_per_thread = popt_args.loop / nthreads;
    threads = (pthread_t*) malloc(nthreads * sizeof(pthread_t));
    calltimes = (struct timespec**) malloc(nthreads * sizeof(struct timespec*));
    for(i = 0; i < nthreads; i++) {
        calltimes[i] = (struct timespec*) malloc(loops_per_thread * sizeof(struct timespec));
    }
    pthread_args = (struct thread_args*) malloc(nthreads
                                            * sizeof(struct thread_args));

    pthread_barrier_init(&barrier, NULL, nthreads);
    for(i = 0; i < nthreads; i++) {
        pthread_args[i].id = i;
        pthread_args[i].ntimes = loops_per_thread;
        pthread_args[i].calltimes = calltimes[i];
        pthread_create(&threads[i], NULL, do_work, (void*)&pthread_args[i]);
	}

    for(i = 0; i < nthreads; i++) {
		pthread_join(threads[i], NULL);
	}

    dump(popt_args.outfile, calltimes);

    pthread_barrier_destroy(&barrier);
    free(threads);
    for(i = 0; i < nthreads; i++) {
       free(calltimes[i]);
    }
    free(calltimes);
    free(pthread_args);

	return ret;
}
