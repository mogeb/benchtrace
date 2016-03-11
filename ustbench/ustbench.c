#define TRACEPOINT_DEFINE
#include "tp.h"

int main()
{
    int i;

    for(i = 0; i < 100000; i++) {
        tracepoint(TRACEPOINT_PROVIDER, bench_tp_4b, 0);
    }
    return 0;
}
