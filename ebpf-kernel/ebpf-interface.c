#include <uapi/linux/ptrace.h>


// map_type, key_type, leaf_type, table_name, num_entry
//BPF_TABLE("hash", struct key_t, u64, stats, 1024);
// trace: <timestamp, payload>
BPF_TABLE("hash", u64, u32, trace, 100000);
//BPF_HASH(mytrace, u64, u32);

int connect_tp(struct pt_regs *ctx, void *p)
{
    u64 ts;
    u32 payload;

    ts = bpf_ktime_get_ns();
    payload = 0;
    trace.lookup_or_init(&ts, &payload);

    return 0;
}

