#undef TRACE_SYSTEM
#define TRACE_SYSTEM empty_tp

#if !defined(_TRACE_EMPTY_MODULE_H) || defined(TRACE_HEADER_MULTI_READ)
#define  _TRACE_EMPTY_MODULE_H

#include <linux/tracepoint.h>

TRACE_EVENT(
    empty_ioctl_4b,
    TP_PROTO(int payload),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __field(int, payload)
    ),
    TP_fast_assign(
        entry->payload = payload
    ),
    TP_printk("payload = %d", __entry->payload)
)

TRACE_EVENT(
    empty_ioctl_8b,
    TP_PROTO(int p1, int p2),
    TP_ARGS(p1, p2),
    TP_STRUCT__entry(
        __field(int, p1)
        __field(int, p2)
    ),
    TP_fast_assign(
        entry->p1 = p1,
        entry->p2 = p2
    ),
    TP_printk("p1 = %d, p2 = %d", __entry->p1, __entry->p2)
)

TRACE_EVENT(
    empty_ioctl_16b,
    TP_PROTO(int p1, int p2, int p3, int p4),
    TP_ARGS(p1, p2, p3, p4),
    TP_STRUCT__entry(
        __field(int, p1)
        __field(int, p2)
        __field(int, p3)
        __field(int, p4)
    ),
    TP_fast_assign(
        entry->p1 = p1,
        entry->p2 = p2,
        entry->p3 = p3,
        entry->p4 = p4
    ),
    TP_printk("p1 = %d, p2 = %d", __entry->p1, __entry->p2)
)

#endif /* _TRACE_EMPTY_MODULE_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .

/* this part must be outside protection */
#include <trace/define_trace.h>
