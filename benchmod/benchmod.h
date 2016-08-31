#ifndef BENCHMOD_H
#define BENCHMOD_H

struct tracker_measurement_entry {
    u64 pmu1;
    u64 pmu2;
    u64 pmu3;
    u64 pmu4;
    u64 latency;
};

struct tracker_measurement_cpu_perf {
    struct perf_event *event1;
    struct perf_event *event2;
    struct perf_event *event3;
    struct perf_event *event4;
    struct tracker_measurement_entry *entries;
    unsigned int pos;
};

#endif // BENCHMOD_H

