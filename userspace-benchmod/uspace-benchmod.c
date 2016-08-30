#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <popt.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ioctl.h>

#include "../benchmod/benchmod.h"

#define BENCHMOD_NAME "/proc/benchmod"

struct popt_args {
    int clear;
    int loop;
    int nthreads;
    int tp_size;
    char *outfile;
    char *tracer;
};

struct popt_args popt_args;

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
        &popt_args.outfile, 0,
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
        &popt_args.nthreads, 0, "Nthreads"
    },
    {
        "clear", 'c',
        POPT_ARG_NONE | POPT_ARGFLAG_OPTIONAL,
        &popt_args.clear, 0,
        "Clear file", "clear"
    },
    {
        "size", 's',
        POPT_ARG_INT | POPT_ARGFLAG_OPTIONAL,
        &popt_args.tp_size, 0,
        "Clear file", "clear"
    },
    POPT_AUTOHELP
};

pthread_barrier_t barrier;

struct ioctl_args {
    int fd;
    int ntimes;
    int id;
};

int copy(const char *from, const char *to)
{
    int fd_to, fd_from;
    char buf[4194304];
    ssize_t nread;
    int saved_errno;

    fd_from = open(from, O_RDONLY);
    if (fd_from < 0)
        return -1;

    fd_to = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_to < 0)
        goto out_error;

    while (nread = read(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = write(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (close(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        close(fd_from);

        /* Success! */
        return 0;
    }

  out_error:
    saved_errno = errno;
    printf("Error during copy, error = %s\n", strerror(errno));

    close(fd_from);
    if (fd_to >= 0)
        close(fd_to);

    errno = saved_errno;
    return -1;
}

static inline int do_ioctl_benchmark(int fd, struct benchmod_arg *args)
{
    return ioctl(fd, _IO('m', 0), args);
}

static inline void do_ioctl_read(int fd, struct timespec *times)
{
    ioctl(fd, _IOR('m', 1, struct timespec*), times);
}


static inline void do_empty_ioctl(int fd)
{
    ioctl(fd, _IO('m', 1));
}

struct timespec ts_diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000 + end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
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

static void dump(char *tracer, struct timespec timespan)
{
    FILE *file;
    int fd;
    unsigned long totaltime;
    char huge[1048576];
    char outfile[1024];
    char tmp_str[16];
    strcpy(outfile, tracer);

    totaltime = timespan.tv_sec * 1000000000 + timespan.tv_nsec;

    /* Write summary */
//    file = fopen(strcat(outfile, ".summary"), popt_args.clear ? "w+" : "a+");

//    if(!file) {
//        fprintf(stderr, "Error opening file %s\n", outfile);
//        return;
//    }

//    if(popt_args.clear) {
//        fprintf(file, "number of threads,total time,number of events\n");
//    }
//    fprintf(file, "%d,%ld,%d\n", popt_args.nthreads, totaltime,
//            popt_args.loop);
//    fprintf(stdout, "%ld\n", timespan.tv_nsec);

//    fclose(file);

//    file = fopen(outfile, "w+");
    fd = open(BENCHMOD_NAME, O_RDONLY);

    if(fd == -1) {
        printf("Error opening the file %s\n", BENCHMOD_NAME);
    }

    /* Dump the content of /proc/benchmod */
    while(read(fd, huge, 1048576) > 0) {
//        fprintf(file, "%s", huge);
    }

    close(fd);
//    fclose(file);

    /* Write raw values for histogram */
    strcpy(outfile, tracer);
    sprintf(tmp_str, "_%dbytes", popt_args.tp_size);
    strcat(outfile, tmp_str);
    sprintf(tmp_str, "_%dprocess", popt_args.nthreads);
    strcat(outfile, tmp_str);
    strcat(outfile, ".hist");
//    copy("/tmp/out.csv", outfile);
//    char cmd[1024];
//    strcat(cmd, "cp /tmp/out.csv ");
//    strcat(cmd, outfile);
//    system(cmd);

}

void *do_work(void *args)
{
    struct ioctl_args *arg = args;
    int i, fd, ret;
    struct benchmod_arg benchmod_args;

    benchmod_args.loop = arg->ntimes;
    benchmod_args.tp_size = popt_args.tp_size;

    fd = open(BENCHMOD_NAME, O_RDONLY);

    pthread_barrier_wait(&barrier);
    /* This ioctl will write a tracepoint in a tight loop */
    ret = do_ioctl_benchmark(fd, &benchmod_args);
    if(ret) {
        printf("Error ioctl %d, %s\n", 0, strerror(errno));
    }
    close(fd);
}

int main(int argc, char **argv)
{
    popt_args.outfile = "none.out";
    popt_args.clear = 0;
    popt_args.tracer = "none";
    popt_args.nthreads = 1;
    popt_args.loop = 1;
    popt_args.tp_size = 4;

    int i, fd, ret = 0;
    struct timespec ts_beg, ts_end, ts_elapsed;
    struct ioctl_args ioctl_arg;
    poptContext pc;
    pthread_t *threads;
    int ncpus = sysconf(_SC_NPROCESSORS_ONLN);

    parse_args(argc, argv, &pc);
    printf("nthreads = %d, loop = %d\n", popt_args.nthreads, popt_args.loop);

    threads = (pthread_t*)malloc(popt_args.nthreads * sizeof(pthread_t));

    if(fd == -1) {
        printf("Error opening %s\n", BENCHMOD_NAME);
        return 1;
    }

    ioctl_arg.fd = fd;
    ioctl_arg.ntimes = popt_args.loop;

    pthread_barrier_init(&barrier, NULL, popt_args.nthreads + 1);
    for(i = 0; i < popt_args.nthreads; i++) {
        ioctl_arg.id = i;
        /* Set CPU affinity for each thread*/
        cpu_set_t cpu_set;
        pthread_attr_t attr;
        CPU_ZERO(&cpu_set);
        CPU_SET(i % ncpus, &cpu_set);
        pthread_attr_init(&attr);
        pthread_attr_setaffinity_np(&attr, sizeof(cpu_set), &cpu_set);

        pthread_create(&threads[i], &attr, do_work, (void*)&ioctl_arg);
    }

    pthread_barrier_wait(&barrier);
    clock_gettime(CLOCK_MONOTONIC, &ts_beg);
    for(i = 0; i < popt_args.nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    pthread_barrier_destroy(&barrier);

    ts_elapsed = ts_diff(ts_beg, ts_end);

    dump(popt_args.tracer, ts_elapsed);

    return ret;
}
