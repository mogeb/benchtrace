#include <stdio.h>
#include "libustperf.h"

void output_measurements()
{
    int i, cpu;
    FILE *outfile;

    outfile = fopen("/tmp/out.csv", "w+");
    fprintf(outfile, "latency,%s,%s,%s,%s\n", metric1_str, metric2_str,
            metric3_str, metric4_str);
    for(cpu = 0; cpu < nCpus; cpu++) {
        for(i = 0; i < cpu_perf[cpu].pos; i++) {
            fprintf(outfile, "%lu,%lu,%lu,%lu,%lu\n",
                    cpu_perf[cpu].entries[i].latency,
                    cpu_perf[cpu].entries[i].pmu1,
                    cpu_perf[cpu].entries[i].pmu2,
                    cpu_perf[cpu].entries[i].pmu3,
                    cpu_perf[cpu].entries[i].pmu4);
        }
    }
    fclose(outfile);
}
