#undef TRACE_SYSTEM
#define TRACE_SYSTEM empty_tp

#if !defined(_TRACE_EMPTY_MODULE_H) || defined(TRACE_HEADER_MULTI_READ)
#define  _TRACE_EMPTY_MODULE_H

#include <linux/tracepoint.h>

#define SIZE_8B 8
#define SIZE_16B 16
#define SIZE_32B 32
#define SIZE_64B 64
#define SIZE_128B 128
#define SIZE_192B 192
#define SIZE_256B 256

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
    TP_PROTO(char payload[SIZE_8B]),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __array(char, payload, SIZE_8B)
    ),
    TP_fast_assign(
        memcpy(__entry->payload, payload, SIZE_8B);
    ),
    TP_printk("payload size = %d", SIZE_8B)
)

TRACE_EVENT(
    empty_ioctl_16b,
    TP_PROTO(char payload[SIZE_16B]),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __array(char, payload, SIZE_16B)
    ),
    TP_fast_assign(
        memcpy(__entry->payload, payload, SIZE_16B);
    ),
    TP_printk("payload size = %d", SIZE_16B)
)

TRACE_EVENT(
    empty_ioctl_32b,
    TP_PROTO(char payload[SIZE_32B]),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __array(char, payload, SIZE_32B)
    ),
    TP_fast_assign(
        memcpy(__entry->payload, payload, SIZE_32B);
    ),
    TP_printk("payload size = %d", SIZE_32B)
)

TRACE_EVENT(
    empty_ioctl_64b,
    TP_PROTO(char payload[SIZE_64B]),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __array(char, payload, SIZE_64B)
    ),
    TP_fast_assign(
        memcpy(__entry->payload, payload, SIZE_64B);
    ),
    TP_printk("payload size = %d", SIZE_64B)
)

TRACE_EVENT(
    empty_ioctl_128b,
    TP_PROTO(char payload[SIZE_128B]),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __array(char, payload, SIZE_128B)
    ),
    TP_fast_assign(
        memcpy(__entry->payload, payload, SIZE_128B);
    ),
    TP_printk("payload size = %d", SIZE_128B)
)

TRACE_EVENT(
    empty_ioctl_192b,
    TP_PROTO(char payload[SIZE_192B]),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __array(char, payload, SIZE_192B)
    ),
    TP_fast_assign(
        memcpy(__entry->payload, payload, SIZE_192B);
    ),
    TP_printk("payload size = %d", SIZE_192B)
)

TRACE_EVENT(
    empty_ioctl_256b,
    TP_PROTO(char payload[SIZE_256B]),
    TP_ARGS(payload),
    TP_STRUCT__entry(
        __array(char, payload, SIZE_256B)
    ),
    TP_fast_assign(
        memcpy(__entry->payload, payload, SIZE_256B);
    ),
    TP_printk("payload size = %d", SIZE_256B)
)

#endif /* _TRACE_EMPTY_MODULE_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .

/* this part must be outside protection */
#include <trace/define_trace.h>
