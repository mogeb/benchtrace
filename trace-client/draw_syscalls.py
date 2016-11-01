import shutil
from pylab import *
from collections import defaultdict

tracers_colors = { 'none': 'r', 'lttng': 'b', 'ftrace': 'y', 'perf': 'm',
                   'kprobe': 'lightskyblue', 'jprobe': 'teal', 'ebpf': 'darksalmon',
                   'lttng-kprobe': 'cadetblue', 'systemtap': 'black'}


def compile_percentiles_nthreads(args):
    tracers = ['lttng', 'ftrace']
    percentiles = defaultdict(list)
    percentiles['lttng'] = [344, 749, 762, 775, 825, 892]
    percentiles['ftrace'] = [321, 699, 700, 701, 707, 710]
    sizes = [0, 1, 6, 12, 24, 48]
    for tracer in tracers:
        plt.plot(sizes, percentiles[tracer], 'o-', label=tracer, color=tracers_colors[tracer])

    # plt.title(str(int(perc * 100)) + 'th percentiles of tracepoint latency according to the number of threads')
    # plt.title('Averages of tracepoint latency according to the number of threads')
    plt.xlabel('File name length in bytes')
    plt.ylabel('Latency in ns')

    # imgname = 'pertp/' + str(int(perc * 100)) + 'th_' + nprocess + 'proc_' + str(args['buf_size_kb']) + 'subbuf_kb'
    # plt.axis([0, 9, 0, 900])
    plt.legend(bbox_to_anchor=(1, 0), loc=4, fontsize='small', borderaxespad=1)
    plt.show()
    # plt.savefig(imgname + '.png', dpi=100)

if __name__ == '__main__':
    compile_percentiles_nthreads([])
