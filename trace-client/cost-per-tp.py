from _snack import label

import shutil
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.pylab as pylab
import matplotlib
from pylab import *
from matplotlib.font_manager import FontProperties
from collections import defaultdict
from statistics import *
from subprocess import call
from mpl_toolkits.mplot3d import Axes3D


tracers_colors = { 'none': 'r', 'lttng': 'b', 'ftrace': 'y', 'perf': 'm',
                   'kprobe': 'lightskyblue', 'jprobe': 'teal' }

def init(args = None):
    return

def do_work(tracer, tracer_name, args = None):
    loops = str(args['loop'])
    tp_sizes = args['tp_sizes']
    nprocesses = args['nprocesses']
    buf_sizes_kb = args['buf_sizes_kb']

    for buf_size_kb in buf_sizes_kb:
        args['buf_size_kb'] = buf_size_kb
        for tp_size in tp_sizes:
            tp_size_str = int(tp_size)
            if tp_size_str >= 1024:
                tp_size_str = int(tp_size_str / 1024)
                tp_size_str = str(tp_size_str) + 'k'
            tp_size_str = str(tp_size_str)
            for i in nprocesses:
                args['tp_size'] = tp_size_str
                tracer.start_tracing('session-test', args)
                if tracer_name == 'perf':
                    call("perf record -e 'empty_tp:empty_ioctl_" + tp_size_str + "b' /home/mogeb/git/benchtrace/all-calls/bin/allcalls -t "
                         + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out" + " -s " + tp_size, shell=True)
                else:
                    call("/home/mogeb/git/benchtrace/all-calls/bin/allcalls -t "
                         + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out" + " -s " + tp_size, shell=True)
                tracer.stop_tracing('session-test')
                shutil.copyfile('/tmp/out.csv', tracer_name + '_' + tp_size + 'bytes_' + buf_size_kb
                                + 'kbsubbuf_' + i + '_process.hist')

def cleanup(args = None):
    return


def compile_results(args):
    # compile_percentiles(args)
    # compile_percentiles_nthreads(args)
    # compile_histograms(args)
    # compile_scatter_plot(args)
    # compile_scatter_plot_CPI(args)
    compile_lttng_subbuf(args)
    return


def compile_percentiles_nthreads(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    nprocesses = args['nprocesses']
    tracers = args['tracers']
    buf_sizes_kb = args['buf_sizes_kb']
    tp_size = 4
    percs = [0.5, 0.75, 0.9, 0.95]

    for buf_size_kb in buf_sizes_kb:
        for perc in percs:
            percentiles = defaultdict(list)
            for tracer in tracers:
                for nprocess in nprocesses:
                    # fname = res_dir + tracer + '_' + str(tp_size) + 'bytes_' + nprocess + 'process.hist'
                    fname = tracer + '_' + str(tp_size) + 'bytes_' + buf_size_kb\
                            + 'kbsubbuf_' + nprocess + '_process.hist'
                    with open(fname, 'r') as f:
                        legend = f.readline()
                    legend = legend.split(',')
                    values = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None, invalid_raise=False)
                    percentiles[tracer].append(np.percentile(values['latency'], perc))
                    # percentiles[tracer].append(np.average(values['latency']))
                plt.plot(nprocesses, percentiles[tracer], 'o-', label=tracer)

            plt.title(str(int(perc * 100)) + 'th percentiles of tracepoint latency according to the number of threads')
            plt.xlabel('Number of threads')
            plt.ylabel('Latency in ns')
            fontP = FontProperties()
            fontP.set_size('small')

            imgname = 'pertp/' + str(int(perc * 100)) + 'th_' + nprocess + 'proc_' + str(args['buf_size_kb']) + 'subbuf_kb'
            plt.axis([0, 9, 0, 900])
            plt.legend(prop=fontP, loc='upper left')
            plt.show()
            # plt.savefig(imgname + '.png', dpi=100)


"""
Will analyze the .hist files.
"""
def compile_percentiles(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    tp_sizes = args['tp_sizes']
    nprocesses = args['nprocesses']
    tracers = args['tracers']
    buf_sizes_kb = args['buf_sizes_kb']
    perc = 0.90

    for buf_size_kb in buf_sizes_kb:
        for nprocess in nprocesses:
            for tracer in tracers:
                percentiles = []
                for tp_size in tp_sizes:
                    # fname = res_dir + tracer + '_' + str(tp_size) + 'bytes_' + nprocess + 'process.hist'
                    fname = tracer + '_' + str(tp_size) + 'bytes_' + buf_size_kb\
                            + 'kbsubbuf_' + nprocess + '_process.hist'
                    with open(fname, 'r') as f:
                        legend = f.readline()
                    legend = legend.split(',')
                    values = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None, invalid_raise=False)
                    percentiles.append(np.percentile(values['latency'], perc))
                plt.plot(tp_sizes, percentiles, 'o-', label=tracer, color=tracers_colors[tracer])

            plt.title(str(int(perc * 100)) + 'th percentiles for the cost of a tracepoint according to payload size')
            plt.xlabel('Payload size in bytes')
            plt.ylabel('Time in ns')
            fontP = FontProperties()
            fontP.set_size('small')

            # imgname = 'pertp/90th_' + nprocess + 'proc_' + str(args['buf_size_kb']) + 'subbuf_kb'
            plt.legend()
            plt.show()
            # plt.savefig(imgname + '.png', dpi=100)


def compile_scatter_plot_ICL(args):
    bytes = args['tp_sizes'][0]
    tracers = args['tracers']
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    values = defaultdict(list)
    cpi = defaultdict(list)

    fname = res_dir + 'none_' + str(bytes) + 'bytes_1process.hist'
    with open(fname, 'r') as f:
        legend = f.readline()
    legend = legend.split(',')

    for tracer in tracers:
        fname = res_dir + tracer + '_' + str(bytes) + 'bytes_1process.hist'
        values[tracer] = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=int)

    for tracer in tracers:
        for i in range(0, len(values[tracer])):
            cpi[tracer].append(values[tracer]['CPU_cycles'][i] / values[tracer]['Instructions'][i])

    for tracer in tracers:
        plt.scatter(values[tracer]['L1_misses'], cpi[tracer], s=values[tracer]['latency']*0.8, color=tracers_colors[tracer], alpha=0.3, label=tracer)

    fontP = FontProperties()
    fontP.set_size('small')
    plt.title('Latency according to CPI and L1 misses')
    plt.xlabel('L1_misses')
    plt.ylabel('CPI')
    plt.legend(prop=fontP)
    plt.show()


def compile_scatter_plot_CPI(args):
    bytes = args['tp_sizes'][0]
    tracers = args['tracers']
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    values = defaultdict(list)
    cpi = defaultdict(list)
    ipc = defaultdict(list)
    handles = defaultdict(list)

    fname = res_dir + 'none_' + str(bytes) + 'bytes_1process.hist'
    with open(fname, 'r') as f:
        legend = f.readline()
    legend = legend.split(',')
    i = 0
    for tracer in tracers:
        fname = res_dir + tracer + '_' + str(bytes) + 'bytes_1process.hist'
        values[tracer] = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=int)

    fontP = FontProperties()
    fontP.set_size('small')

    for tracer in tracers:
        for i in range(0, len(values[tracer])):
            ipc[tracer].append(values[tracer]['Instructions'][i] / values[tracer]['CPU_cycles'][i])
            cpi[tracer].append(values[tracer]['CPU_cycles'][i] / values[tracer]['Instructions'][i])

    plt.subplot(2, 1, 0)
    plt.title('Latency according to CPI')
    plt.xlabel('CPI')
    plt.ylabel('Latency in ns')
    plt.legend(prop=fontP)
    for tracer in tracers:
        plt.scatter(cpi[tracer], values[tracer]['latency'], color=tracers_colors[tracer], alpha=0.3, label=tracer)
    plt.legend(prop=fontP)

    plt.subplot(2, 1, 1)
    plt.title('Latency according to IPC')
    plt.xlabel('IPC')
    plt.ylabel('Latency in ns')
    plt.legend(prop=fontP)
    for tracer in tracers:
        plt.scatter(ipc[tracer], values[tracer]['latency'], color=tracers_colors[tracer], alpha=0.3, label=tracer)

    plt.legend(prop=fontP)
    plt.show()


def compile_scatter_plot(args):
    bytes = args['tp_sizes'][0]
    tracers = args['tracers']
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    values = defaultdict(list)
    fname = res_dir + 'none_' + str(bytes) + 'bytes_1process.hist'
    with open(fname, 'r') as f:
        legend = f.readline()
    legend = legend.split(',')
    i = 0
    for tracer in tracers:
        fname = res_dir + tracer + '_' + str(bytes) + 'bytes_1process.hist'
        values[tracer] = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None)

    fontP = FontProperties()
    fontP.set_size('small')
    for metric in legend:
        plt.subplot(2, 2, i)
        for tracer in tracers:
            if metric == 'latency':
                continue
            metric = metric.strip()
            plt.scatter(values[tracer][metric], values[tracer]['latency'], color=tracers_colors[tracer], alpha=0.3, label=tracer)
        plt.title('Latency according to ' + metric)
        plt.xlabel(metric)
        plt.ylabel('Latency in ns')
        plt.legend(prop=fontP)
        i += 1
    plt.show()


"""
Will analyze the .hist files.
"""
def compile_lttng_subbuf(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    tp_sizes = args['tp_sizes']
    nprocesses = args['nprocesses']
    tracer = 'lttng'
    buf_sizes_kb = args['buf_sizes_kb']
    perc = 0.90

    for tp_size in tp_sizes:
        percentiles = []
        for buf_size_kb in buf_sizes_kb:
            for tp_size in tp_sizes:
                # fname = res_dir + tracer + '_' + str(byte_size) + 'bytes_' + nprocess + 'process.hist'
                fname = tracer + '_' + str(tp_size) + 'bytes_' + buf_size_kb\
                        + 'kbsubbuf_1_process.hist'
                with open(fname, 'r') as f:
                    legend = f.readline()
                legend = legend.split(',')
                values = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None, invalid_raise=False)
                # percentiles.append(np.percentile(values['latency'], perc))
                percentiles.append(np.average(values['latency']))
        plt.plot(buf_sizes_kb, percentiles, 'o-', label=tracer, color=tracers_colors[tracer])

        plt.title(str(int(perc * 100)) + 'th percentiles for the cost of a tracepoint according to payload size')
        plt.xlabel('Subbuffer size in kb')
        plt.ylabel('Time in ns')
        fontP = FontProperties()
        fontP.set_size('small')

        # imgname = 'pertp/90th_' + nprocess + 'proc_' + str(args['buf_size_kb']) + 'subbuf_kb'
        plt.legend()
        plt.show()
        # plt.savefig(imgname + '.png', dpi=100)


def compile_scatter_plot_3d(args):
    bytes = args['tp_sizes'][0]
    tracers = args['tracers']
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    values = defaultdict(list)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    fname = res_dir + 'none_' + str(bytes) + 'bytes_1process.hist'
    with open(fname, 'r') as f:
        legend = f.readline()
    legend = legend.split(',')
    i = 0
    for tracer in tracers:
        fname = res_dir + tracer + '_' + str(bytes) + 'bytes_1process.hist'
        values[tracer] = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None)

    fontP = FontProperties()
    fontP.set_size('small')

    for tracer in tracers:
        ax.scatter(values[tracer]['cache_misses'], values[tracer]['Instructions'], values[tracer]['latency'])
    plt.title('Latency according to cache_misses and instructions')
    ax.set_xlabel('cache_misses')
    ax.set_ylabel('Instructions')
    ax.set_zlabel('latency')
    plt.legend(prop=fontP)
    plt.show()


def compile_histograms(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    tracers = args['tracers']
    tp_size = args['tp_sizes'][0]
    nbins = 100

    for tracer in tracers:
        fname = res_dir + tracer + '_' + str(tp_size) + 'bytes_1process.hist'
        with open(fname, 'r') as f:
            legend = f.readline()
        legend = legend.split(',')
        values = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None)
        plt.hist(values['latency'], bins=nbins, color=tracers_colors[tracer], alpha=0.5, label=tracer)
    plt.legend()
    plt.show()


def is_outlier(points, thresh=500000000):
    if len(points.shape) == 1:
        points = points[:,None]
    median = np.median(points, axis=0)
    diff = np.sum((points - median)**2, axis=-1)
    diff = np.sqrt(diff)
    med_abs_deviation = np.median(diff)

    modified_z_score = 0.6745 * diff / med_abs_deviation

    return modified_z_score > thresh


def rstyle(ax):
    """Styles an axes to appear like ggplot2
    Must be called after all plot and axis manipulation operations have been carried out (needs to know final tick spacing)
    """
    #set the style of the major and minor grid lines, filled blocks
    ax.grid(True, 'major', color='w', linestyle='-', linewidth=1.4)
    ax.grid(True, 'minor', color='0.92', linestyle='-', linewidth=0.7)
    ax.patch.set_facecolor('0.85')
    ax.set_axisbelow(True)

    #set minor tick spacing to 1/2 of the major ticks
    ax.xaxis.set_minor_locator(MultipleLocator( (plt.xticks()[0][1]-plt.xticks()[0][0]) / 2.0 ))
    ax.yaxis.set_minor_locator(MultipleLocator( (plt.yticks()[0][1]-plt.yticks()[0][0]) / 2.0 ))

    #remove axis border
    for child in ax.get_children():
        if isinstance(child, matplotlib.spines.Spine):
            child.set_alpha(0)

    #restyle the tick lines
    for line in ax.get_xticklines() + ax.get_yticklines():
        line.set_markersize(5)
        line.set_color("gray")
        line.set_markeredgewidth(1.4)

    #remove the minor tick lines
    for line in ax.xaxis.get_ticklines(minor=True) + ax.yaxis.get_ticklines(minor=True):
        line.set_markersize(0)

    #only show bottom left ticks, pointing out of axis
    rcParams['xtick.direction'] = 'out'
    rcParams['ytick.direction'] = 'out'
    ax.xaxis.set_ticks_position('bottom')
    ax.yaxis.set_ticks_position('left')


    if ax.legend_ != None:
        lg = ax.legend_
        lg.get_frame().set_linewidth(0)
        lg.get_frame().set_alpha(0.5)


def rhist(ax, data, **keywords):
    """Creates a histogram with default style parameters to look like ggplot2
    Is equivalent to calling ax.hist and accepts the same keyword parameters.
    If style parameters are explicitly defined, they will not be overwritten
    """

    defaults = {
                'facecolor' : '0.3',
                'edgecolor' : '0.28',
                'linewidth' : '1',
                'bins' : 100
                }

    for k, v in defaults.items():
        if k not in keywords: keywords[k] = v

    return ax.hist(data, **keywords)


def rbox(ax, data, **keywords):
    """Creates a ggplot2 style boxplot, is eqivalent to calling ax.boxplot with the following additions:

    Keyword arguments:
    colors -- array-like collection of colours for box fills
    names -- array-like collection of box names which are passed on as tick labels

    """

    hasColors = 'colors' in keywords
    if hasColors:
        colors = keywords['colors']
        keywords.pop('colors')

    if 'names' in keywords:
        ax.tickNames = plt.setp(ax, xticklabels=keywords['names'] )
        keywords.pop('names')

    bp = ax.boxplot(data, **keywords)
    pylab.setp(bp['boxes'], color='black')
    pylab.setp(bp['whiskers'], color='black', linestyle = 'solid')
    pylab.setp(bp['fliers'], color='black', alpha = 0.9, marker= 'o', markersize = 3)
    pylab.setp(bp['medians'], color='black')

    numBoxes = len(data)
    for i in range(numBoxes):
        box = bp['boxes'][i]
        boxX = []
        boxY = []
        for j in range(5):
          boxX.append(box.get_xdata()[j])
          boxY.append(box.get_ydata()[j])
        boxCoords = zip(boxX,boxY)

        if hasColors:
            boxPolygon = Polygon(boxCoords, facecolor = colors[i % len(colors)])
        else:
            boxPolygon = Polygon(boxCoords, facecolor = '0.95')

        ax.add_patch(boxPolygon)
    return bp


