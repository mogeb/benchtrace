#undef TRACEPOINT_PROVIDER
#define TRACEPOINT_PROVIDER ustbench

#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_INCLUDE "./tp.h"

#if !defined(_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define _TP_H

#include <lttng/tracepoint.h>

TRACEPOINT_EVENT(
    ustbench,
    bench_tp_4b,
    TP_ARGS(
        int, p1
    ),
    TP_FIELDS(
        ctf_integer(int, my_integer_field, p1)
    )
)

#endif /* _TP_H */

#include <lttng/tracepoint-event.h>
