#if !defined(MEASURE_H)
#define MEASURE_H

/*
 * Copyright (C) 2016 Julien Desfossez <jdesfossez@efficios.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/perf_event.h>
#include "wrapper/vmalloc.h"

#define NBUCKETS 100

#define irq_stats(x)            (&per_cpu(irq_stat, x))

//#define PER_CPU_ALLOC 10000
struct tracker_measurement_entry {
    u64 pmu1;
    u64 pmu2;
    u64 pmu3;
    u64 pmu4;
};

struct tracker_measurement_cpu_perf {
    struct perf_event *event1;
    struct perf_event *event2;
    struct perf_event *event3;
    struct perf_event *event4;
    struct tracker_measurement_entry *entries;
};

struct perf_event_attr attr1, attr2, attr3, attr4;

static struct tracker_measurement_cpu_perf __percpu *tracker_cpu_perf;

#define BENCH_PREAMBULE unsigned long _bench_flags; \
        unsigned int _bench_nmi; \
        int _bench_pos; \
        int *_bench_h; \
        unsigned long *_bench_a; \
        struct tracker_measurement_cpu_perf *_bench_c; \
        struct timespec _bench_ts1, _bench_ts2, _bench_diff; \
        u64 _bench_pmu1_1 = 0, _bench_pmu1_2 = 0; \
        u64 _bench_pmu2_1 = 0, _bench_pmu2_2 = 0; \
        u64 _bench_pmu3_1 = 0, _bench_pmu3_2 = 0; \
        u64 _bench_pmu4_1 = 0, _bench_pmu4_2 = 0; \
        _bench_c = this_cpu_ptr(tracker_cpu_perf); \
        _bench_a = this_cpu_ptr(&averages); \
        _bench_h = this_cpu_ptr(histos); \
        local_irq_save(_bench_flags); \
        _bench_nmi = irq_stats(smp_processor_id())->__nmi_count

#define BENCH_GET_TS1 _bench_c->event1->pmu->read(_bench_c->event1); \
        _bench_c->event2->pmu->read(_bench_c->event2); \
        _bench_c->event3->pmu->read(_bench_c->event3); \
        _bench_c->event4->pmu->read(_bench_c->event4); \
        _bench_pmu1_1 = local64_read(&_bench_c->event1->count); \
        _bench_pmu2_1 = local64_read(&_bench_c->event2->count); \
        _bench_pmu3_1 = local64_read(&_bench_c->event3->count); \
        _bench_pmu4_1 = local64_read(&_bench_c->event4->count); \
        getnstimeofday(&_bench_ts1);

#define BENCH_GET_TS2 getnstimeofday(&_bench_ts2); \
        _bench_c->event1->pmu->read(_bench_c->event1); \
        _bench_c->event2->pmu->read(_bench_c->event2); \
        _bench_c->event3->pmu->read(_bench_c->event3); \
        _bench_c->event4->pmu->read(_bench_c->event4); \
        _bench_pmu1_2 = local64_read(&_bench_c->event1->count); \
        _bench_pmu2_2 = local64_read(&_bench_c->event2->count); \
        _bench_pmu3_2 = local64_read(&_bench_c->event3->count); \
        _bench_pmu4_2 = local64_read(&_bench_c->event4->count); \
        if (_bench_nmi == irq_stats(smp_processor_id())->__nmi_count) { \
        _bench_diff = do_ts_diff(_bench_ts1, _bench_ts2); \
        if(_bench_diff.tv_nsec > NBUCKETS * hist_granularity_ns) { \
            _bench_pos = (NBUCKETS * hist_granularity_ns - 1) / hist_granularity_ns; \
        } else { \
            _bench_pos = _bench_diff.tv_nsec / hist_granularity_ns; \
        } \
        _bench_h[_bench_pos]++; \
        *_bench_a += _bench_diff.tv_sec * BILLION + (unsigned long)_bench_diff.tv_nsec; \
        _bench_c->entries[_bench_pos].pmu1 += _bench_pmu1_2 - _bench_pmu1_1; \
        _bench_c->entries[_bench_pos].pmu2 += _bench_pmu2_2 - _bench_pmu2_1; \
        _bench_c->entries[_bench_pos].pmu3 += _bench_pmu3_2 - _bench_pmu3_1; \
        _bench_c->entries[_bench_pos].pmu4 += _bench_pmu4_2 - _bench_pmu4_1; \
        }

#define BENCH_APPEND \
    local_irq_restore(_bench_flags)

static
void overflow_callback(struct perf_event *event,
        struct perf_sample_data *data,
        struct pt_regs *regs)
{
}

static
int alloc_measurements(void)
{
    int cpu, ret;
    struct tracker_measurement_cpu_perf *c;

    tracker_cpu_perf = alloc_percpu(struct tracker_measurement_cpu_perf);

    /* include/uapi/linux/perf_event.h */
    /* attr1 = L1-dcache-load-misses */
    attr1.size = sizeof(struct perf_event_attr);
    attr1.pinned = 1;
    attr1.disabled = 0;
    attr1.type = PERF_TYPE_HW_CACHE;
    attr1.config = PERF_COUNT_HW_CACHE_L1D | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;

    /* attr2 = LLC-load-misses */
    attr2.size = sizeof(struct perf_event_attr);
    attr2.pinned = 1;
    attr2.disabled = 0;
    attr2.type = PERF_TYPE_HW_CACHE;
    attr2.config = PERF_COUNT_HW_CACHE_LL | \
               PERF_COUNT_HW_CACHE_OP_READ << 8 | \
               PERF_COUNT_HW_CACHE_RESULT_MISS << 16;

    /* attr3 = cache misses */
    attr3.size = sizeof(struct perf_event_attr);
    attr3.pinned = 1;
    attr3.disabled = 0;
    attr3.type = PERF_TYPE_HARDWARE;
    attr3.config = PERF_COUNT_HW_CACHE_MISSES;

    /* attr4 = instructions */
    attr4.size = sizeof(struct perf_event_attr);
    attr4.pinned = 1;
    attr4.disabled = 0;
    attr4.type = PERF_TYPE_HARDWARE;
    attr4.config = PERF_COUNT_HW_INSTRUCTIONS;

    for_each_online_cpu(cpu) {
        c = per_cpu_ptr(tracker_cpu_perf, cpu);
        c->entries = vzalloc(NBUCKETS * sizeof(struct tracker_measurement_entry));
        if (!c->entries) {
            ret = -ENOMEM;
            goto end;
        }

        c->event1 = perf_event_create_kernel_counter(&attr1,
                cpu, NULL, overflow_callback, NULL);
        if (!c->event1) {
            printk("failed to create perf counter\n");
            ret = -1;
            goto end;
        }

        c->event2 = perf_event_create_kernel_counter(&attr2,
                cpu, NULL, overflow_callback, NULL);
        if (!c->event2) {
            printk("failed to create perf counter\n");
            ret = -1;
            goto end;
        }
        c->event3 = perf_event_create_kernel_counter(&attr3,
                cpu, NULL, overflow_callback, NULL);
        if (!c->event3) {
            printk("failed to create perf counter\n");
            ret = -1;
            goto end;
        }
        c->event4 = perf_event_create_kernel_counter(&attr4,
                cpu, NULL, overflow_callback, NULL);
        if (!c->event4) {
            printk("failed to create perf counter\n");
            ret = -1;
            goto end;
        }
    }
    ret = 0;

end:
    wrapper_vmalloc_sync_all();
    return ret;
}

static
void output_measurements(void)
{
    int cpu, i;
    loff_t pos = 0;
    struct file *file;
    mm_segment_t old_fs;
    char buf[256];
    struct tracker_measurement_cpu_perf tmp_measurement;

    old_fs = get_fs();
    set_fs(get_ds());

    file = filp_open("/tmp/out.csv", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (!file) {
        printk("Failed to open the output file\n");
        goto end;
    }

    tmp_measurement.entries = vzalloc(NBUCKETS *
                                      sizeof(struct tracker_measurement_entry));
    for_each_online_cpu(cpu) {
        struct tracker_measurement_cpu_perf *_bench_c;
        _bench_c = per_cpu_ptr(tracker_cpu_perf, cpu);
        for (i = 0; i < NBUCKETS; i++) {
            tmp_measurement.entries[i].pmu1 += _bench_c->entries[i].pmu1;
            tmp_measurement.entries[i].pmu2 += _bench_c->entries[i].pmu2;
            tmp_measurement.entries[i].pmu3 += _bench_c->entries[i].pmu3;
            tmp_measurement.entries[i].pmu4 += _bench_c->entries[i].pmu4;
        }
    }

    snprintf(buf, 256, "L1_miss,LLC_miss,cache_misses,instructions\n");
    vfs_write(file, buf, strlen(buf), &pos);
    for (i = 0; i < NBUCKETS; i++) {
        snprintf(buf, 256, "%llu,%llu,%llu,%llu\n",
                 tmp_measurement.entries[i].pmu1,
                 tmp_measurement.entries[i].pmu2,
                 tmp_measurement.entries[i].pmu3,
                 tmp_measurement.entries[i].pmu4);
        vfs_write(file, buf, strlen(buf), &pos);
    }
    filp_close(file, NULL);

end:
    set_fs(old_fs); //Reset to save FS
    return;
}

static
void free_measurements(void)
{
    int cpu;
    struct tracker_measurement_cpu_perf *c;

    for_each_online_cpu(cpu) {
        c = per_cpu_ptr(tracker_cpu_perf, cpu);
        perf_event_release_kernel(c->event1);
        perf_event_release_kernel(c->event2);
        perf_event_release_kernel(c->event3);
        perf_event_release_kernel(c->event4);
        vfree(c->entries);
    }
    free_percpu(tracker_cpu_perf);
}


#endif /* MEASURE_H */
