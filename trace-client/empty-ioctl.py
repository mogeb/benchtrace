import numpy as np
import matplotlib.pyplot as plt
import matplotlib.pylab as pylab
import os
from collections import defaultdict
from statistics import *
from subprocess import call

def init(args = None):
    return

def do_work(tracer, tracer_name, args = None):
    loops = str(args['loop'])
    tp_sizes = args['tp_sizes']
    nprocess = args['nprocess']

    for tp_size in tp_sizes:
        for i in nprocess:
            args['tp_size'] = tp_size
            tracer.start_tracing('session-test', args)
            if tracer_name == 'perf':
                call("perf stat -e 'empty_tp:empty_ioctl_" + tp_size + "b' /home/mogeb/git/benchtrace/all-calls/allcalls -t "
                     + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out" + " -s " + tp_size, shell=True)
            else:
                call("/home/mogeb/git/benchtrace/all-calls/allcalls -t "
                     + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out" + " -s " + tp_size, shell=True)
            tracer.stop_tracing('session-test')

def cleanup(args = None):
    return


def is_outlier(points, thresh=500000000):
    if len(points.shape) == 1:
        points = points[:,None]
    median = np.median(points, axis=0)
    diff = np.sum((points - median)**2, axis=-1)
    diff = np.sqrt(diff)
    med_abs_deviation = np.median(diff)

    modified_z_score = 0.6745 * diff / med_abs_deviation

    return modified_z_score > thresh


def compile_results():
    compile_bars()
    compile_graphs()
    return


def compile_graphs():
    tp_sizes = ['4', '64', '128', '192', '256']
    tracers = ['none', 'lttng', 'ftrace', 'perf']
    values = defaultdict(list)

    """
    Fix thread size = 1
    """
    for tracer in tracers:
        for tp_size in tp_sizes:
            fname = tracer + '_' + tp_size + 'bytes_1process.hist'
            if not os.path.isfile(fname):
                continue
            with open(fname, 'rb') as file:
                for line in file:
                    pass
                values[tracer].append(line)
    print(values)
    for tracer in tracers:
        if not len(values[tracer]) == 0:
            plt.plot(tp_sizes, values[tracer], 'o-', label=tracer)
    plt.axis()
    plt.ylabel('some numbers')
    plt.legend()
    plt.show()

    return


def compile_bars():
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    width = 0.15       # the width of the bars

    # for bytes in ['4', '8', '16', '32', '64', '128', '192', '256']:
    for bytes in ['4', '64', '128', '192', '256']:
        lttng_values = np.genfromtxt(res_dir + 'lttng_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                      names=['min', 'max', 'num'], dtype=None, skip_footer=1)

        N = len(lttng_values['num'])
        ind = np.arange(N)  # the x locations for the groups

        fig, ax = plt.subplots()

        none_values = np.genfromtxt(res_dir + 'none_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                      names=['min', 'max', 'num'], dtype=None, skip_footer=1)
        ftrace_values = np.genfromtxt(res_dir + 'ftrace_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                      names=['min', 'max', 'num'], dtype=None, skip_footer=1)
        perf_values = np.genfromtxt(res_dir + 'perf_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                      names=['min', 'max', 'num'], dtype=None, skip_footer=1)
        # rect_none = ax.bar(ind, none_values['totaltime'], width, color='b')

        rect_none = ax.bar(ind, none_values['num'], width, color='r')
        rect_lttng = ax.bar(ind + width, lttng_values['num'], width, color='b')
        rect_ftrace = ax.bar(ind + 2 * width, ftrace_values['num'], width, color='y')
        rect_perf = ax.bar(ind + 3 * width, perf_values['num'], width, color='m')

        # add some text for labels, title and axes ticks
        ax.set_ylabel('Time in ns')
        ax.set_title('Time taken to do N calls')
        ax.set_xticks(ind + width)
        ax.set_xticklabels(lttng_values['max'])

        ax.legend((rect_none[0], rect_lttng[0], rect_ftrace[0], rect_perf[0]), ('None', 'LTTng', 'Ftrace', 'Perf'))

        plt.axis([0, 20, 0, 1000000])
        plt.show()
    return


def compile_bars_old():
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

def compile_histograms():
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    # none_values = np.genfromtxt(res_dir + 'none.out', delimiter=',', skip_header=2,
    #               names=['latency'], dtype=None)

    none_values = [ ( pylab.loadtxt(filename) ) for filename in [(res_dir + 'none.hist')] ] [0]
    lttng_values = [ ( pylab.loadtxt(filename) ) for filename in [(res_dir + 'lttng.hist')] ] [0]
    ftrace_values = [ ( pylab.loadtxt(filename) ) for filename in [(res_dir + 'ftrace.hist')] ][0]
    perf_values = [ ( pylab.loadtxt(filename) ) for filename in [(res_dir + 'perf.hist')] ][0]

    print('Mean none: ' + str(mean(none_values.tolist())))
    print('Median none: ' + str(median(none_values.tolist())))
    print('90th per none: ' + str(np.percentile(none_values.tolist(), 90)))
    print('95th per none: ' + str(np.percentile(none_values.tolist(), 95)))
    print()
    print('Mean lttng: ' + str(mean(lttng_values.tolist())))
    print('Median lttng: ' + str(median(lttng_values.tolist())))
    print('90th per lttng: ' + str(np.percentile(lttng_values.tolist(), 90)))
    print('95th per lttng: ' + str(np.percentile(lttng_values.tolist(), 95)))
    print()
    print('Mean ftrace: ' + str(mean(ftrace_values.tolist())))
    print('Median ftrace: ' + str(median(ftrace_values.tolist())))
    print('90th per ftrace: ' + str(np.percentile(ftrace_values.tolist(), 90)))
    print('95th per ftrace: ' + str(np.percentile(ftrace_values.tolist(), 95)))
    print()
    print('Mean perf: ' + str(mean(perf_values.tolist())))
    print('Median perf: ' + str(median(perf_values.tolist())))
    print('90th per perf: ' + str(np.percentile(perf_values.tolist(), 90)))
    print('95th per perf: ' + str(np.percentile(perf_values.tolist(), 95)))

    nbins=1000
    isnormed=False
    iscumul=False
    if isnormed == True:
        plt.axis([30, 400, 0, 1])
    else:
        plt.axis([30, 400, 0, 11000])
    lttng_filtered = lttng_values[~is_outlier(lttng_values)]
    ftrace_filtered = ftrace_values[~is_outlier(ftrace_values)]
    none_filtered = none_values[~is_outlier(none_values)]
    perf_filtered = perf_values[~is_outlier(perf_values)]
    plt.hist(none_filtered.tolist(), normed=isnormed, cumulative=iscumul, bins=nbins, color='y', alpha=0.5, label='none')
    plt.hist(lttng_filtered.tolist(), normed=isnormed, cumulative=iscumul, bins=nbins, color='b', label='lttng')
    plt.hist(ftrace_filtered.tolist(), normed=isnormed, cumulative=iscumul, bins=nbins, color='r', alpha=0.5, label='ftrace')
    plt.hist(perf_filtered.tolist(), normed=isnormed, cumulative=iscumul, bins=nbins, color='g', alpha=0.5, label='perf')
    plt.title("Gaussian Histogram")
    plt.xlabel("Value")
    plt.ylabel("Frequency")
    plt.legend()

    print('none: ' + str(len(none_filtered.tolist())))
    print('lttng: ' + str(len(lttng_filtered.tolist())))
    print('ftrace: ' + str(len(ftrace_filtered.tolist())))
    print('perf: ' + str(len(perf_filtered.tolist())))


    plt.show()


    # plt.xlabel('Smarts')
    # plt.ylabel('Probability')
    # plt.grid(True)
    #
    # plt.show()
