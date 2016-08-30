#define _GNU_SOURCE
#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <time.h>

#define NCPUS 4

enum {
    TRACE_BUF_LEN = 4096 * 64,
    TRACE_BUF_FLUSH_THRESHOLD = TRACE_BUF_LEN / 4,
};

struct trace_t {
    uint8_t trace_buf[TRACE_BUF_LEN];
    uint32_t idx;
};

// Will contain the actual trace
struct trace_t trace[NCPUS];

static unsigned int write_to_buffer(int cpu, void *dataptr, size_t size)
{
    uint8_t *data_ptr = dataptr;
    uint32_t x = 0;
    uint32_t *idx;

    idx = &trace[cpu].idx;

    if (*idx >= TRACE_BUF_LEN) {
        *idx = *idx % TRACE_BUF_LEN;
    }
    trace[cpu].trace_buf[(*idx)++] = data_ptr[x++];

    return *idx;
}

void trace_record_write(void *dataptr, size_t size)
{
    struct timespec ts;
    int cpu = sched_getcpu();

    clock_gettime(CLOCK_MONOTONIC, &ts);
    write_to_buffer(cpu, &ts.tv_sec, sizeof(ts.tv_sec));
    write_to_buffer(cpu, &ts.tv_nsec, sizeof(ts.tv_nsec));
    write_to_buffer(cpu, dataptr, size);
}
