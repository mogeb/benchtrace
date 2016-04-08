#include <uapi/linux/ptrace.h>

struct event {
    u64 ts;
    u32 payload;
};

BPF_PERF_OUTPUT(trace);

int connect_tp(struct pt_regs *ctx, void *p)
{
    u64 ts;
    u32 payload;
    struct event ev = {};

    ts = bpf_ktime_get_ns();
    ev.ts = ts;
    payload = 0;
    ev.payload = payload;
    trace.perf_submit(ctx, &ev, sizeof(ev));

    return 0;
}

