#ifndef UTILS_H
#define UTILS_H

#define MIN(a, b) (a) < (b) ? (a) : (b)

#define PER_CPU_ALLOC 5000
#define METRIC_LEN 32
#define BILLION 1000000000

static inline struct timespec do_ts_diff(struct timespec start,
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

static inline struct timespec do_ts_add(struct timespec t1, struct timespec t2)
{
    long sec = t2.tv_sec + t1.tv_sec;
    long nsec = t2.tv_nsec + t1.tv_nsec;
    if (nsec >= BILLION) {
        nsec -= BILLION;
        sec++;
    }
    return (struct timespec) { .tv_sec = sec, .tv_nsec = nsec };
}

#endif // UTILS_H
