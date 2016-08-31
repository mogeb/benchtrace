#ifndef BENCHMOD_UTILS_H
#define BENCHMOD_UTILS_H

#define PROC_ENTRY_NAME "benchmod"
#define BENCHMARK_MAGIC 'm'

#define IOCTL_BENCHMARK _IO(BENCHMARK_MAGIC, 0)
#define IOCTL_READ_RES  _IOR(BENCHMARK_MAGIC, 1, struct timspec*)
#define IOCTL_EMPTY_CALL _IO(BENCHMARK_MAGIC, 2)

struct benchmod_arg {
    int loop;
    int tp_size;
};

#endif // BENCHMOD_UTILS_H
