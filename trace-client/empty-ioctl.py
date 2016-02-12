import numpy as np
import matplotlib.pyplot as plt
from subprocess import call

def init(args = None):
    return

def do_work(tracer, tracer_name, args = None):
    loops = str(4000)
    for i in [1]:
        tracer.start_tracing('session-test', args)
        if tracer_name == 'perf':
            call("perf stat -e 'empty_tp:*' /home/mogeb/git/benchtrace/all-calls/allcalls -t "
                 + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out", shell=True)
        else:
            call("/home/mogeb/git/benchtrace/all-calls/allcalls -t "
                 + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out", shell=True)
        tracer.stop_tracing('session-test')


def cleanup(args = None):
    return


def compile_results():
    return
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    width = 0.15       # the width of the bars

    lttng_values = np.genfromtxt(res_dir + 'lttng.out', delimiter=',', skip_header=0,
                  names=['nthreads', 'totaltime', 'loop'], dtype=None)

    N = len(lttng_values['totaltime'])
    ind = np.arange(N)  # the x locations for the groups

    fig, ax = plt.subplots()

    none_values = np.genfromtxt(res_dir + 'none.out', delimiter=',', skip_header=0,
                  names=['nthreads', 'totaltime', 'loop'], dtype=None)
    rect_none = ax.bar(ind, none_values['totaltime'], width, color='b')

    rect_lttng = ax.bar(ind + width, lttng_values['totaltime'], width, color='r')

    ftrace_values = np.genfromtxt(res_dir + 'ftrace.out', delimiter=',', skip_header=0,
                  names=['nthreads', 'totaltime', 'loop'], dtype=None)
    rect_ftrace = ax.bar(ind + 2 * width, ftrace_values['totaltime'], width, color='y')

    perf_values = np.genfromtxt(res_dir + 'perf.out', delimiter=',', skip_header=0,
                  names=['nthreads', 'totaltime', 'loop'], dtype=None)
    rect_perf = ax.bar(ind + 3 * width, perf_values['totaltime'], width, color='m')

    # add some text for labels, title and axes ticks
    ax.set_ylabel('Time in ns')
    ax.set_title('Time taken to do N calls')
    ax.set_xticks(ind + width)
    ax.set_xticklabels(lttng_values['nthreads'])

    ax.legend((rect_none[0], rect_lttng[0], rect_ftrace[0], rect_perf[0]), ('None', 'LTTng', 'Ftrace', 'Perf'))

    plt.show()
    return