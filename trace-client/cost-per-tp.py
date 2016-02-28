import numpy as np
import matplotlib.pyplot as plt
import matplotlib.pylab as pylab
import matplotlib
from pylab import *
from matplotlib.font_manager import FontProperties
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
        tp_size_str = int(tp_size)
        if tp_size_str >= 1024:
            tp_size_str = int(tp_size_str / 1024)
            tp_size_str = str(tp_size_str) + 'k'
        tp_size_str = str(tp_size_str)
        for i in nprocess:
            args['tp_size'] = tp_size_str
            tracer.start_tracing('session-test', args)
            if tracer_name == 'perf':
                call("perf record -e 'empty_tp:empty_ioctl_" + tp_size_str + "b' /home/mogeb/git/benchtrace/all-calls/allcalls -t "
                     + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out" + " -s " + tp_size, shell=True)
            else:
                call("/home/mogeb/git/benchtrace/all-calls/allcalls -t "
                     + tracer_name + " -n " + loops + " -p " + str(i) + " -o " + tracer_name + ".out" + " -s " + tp_size, shell=True)
            tracer.stop_tracing('session-test')

def cleanup(args = None):
    return


def compile_results(args):
    compile_percentiles(args)
    compile_bars(args)
    return


"""
Will analyze the .hist files.
"""
def compile_percentiles(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    byte_sizes = ['4', '32', '64', '128', '192', '256', '512', '768', '1024']
    nprocesses = ['1']
    tracers = ['none', 'lttng', 'ftrace', 'perf']
    perc = 0.90

    for nprocess in nprocesses:
        fig = plt.figure(figsize=(14, 7))
        for tracer in tracers:
            percentiles = []
            for bytes in byte_sizes:
                fname = res_dir + tracer + '_' + bytes + 'bytes_' + nprocess + 'process.hist'
                values = np.genfromtxt(fname, delimiter=',', skip_header=0,
                              names=['min', 'max', 'num'], dtype=None, skip_footer=1)
                percentiles.append(getPercentile(values['max'], values['num'], perc))
            ax = fig.add_subplot(1, 1, 1)
            ax.plot(byte_sizes, percentiles, 'o-', label=tracer)
            # plt.plot(byte_sizes, percentiles, 'o-', label=tracer)

        plt.axis([0, 1050, 0, 700])
        plt.title(str(int(perc * 100)) + 'th percentiles for the cost of a tracepoint according to payload size')
        plt.xlabel('Payload size in bytes')
        plt.ylabel('Time in ns')
        fontP = FontProperties()
        fontP.set_size('small')

        imgname = 'pertp/90th_' + nprocess + 'proc_' + str(args['buf_size_kb']) + 'subbuf_kb'
        plt.legend()
        # plt.show()
        plt.savefig(imgname + '.png', dpi=100)

    return values['max'], values['num'], percentiles

def compile_bars(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    width = 0.2       # the width of the bars

    none_percentiles = []
    lttng_percentiles = []
    ftrace_percentiles = []
    perf_percentiles = []
    byte_sizes = ['4', '32', '64', '128', '192', '256', '512', '768', '1024']
    # byte_sizes = ['1024']
    tracers = ['none', 'lttng', 'ftrace', 'perf']
    nprocesses = ['1']

    for nprocess in nprocesses:
        for bytes in byte_sizes:
            lttng_values = np.genfromtxt(res_dir + 'lttng_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                          names=['min', 'max', 'num'], dtype=None, skip_footer=1)
            lttng_percentiles.append(getPercentile(lttng_values['max'], lttng_values['num'], 0.9))

            N = len(lttng_values['num'])
            ind = np.arange(N)
            ticks = np.arange(0, N, 5)  # the x locations for the groups

            fig, ax = plt.subplots(figsize=(14, 7))

            none_values = np.genfromtxt(res_dir + 'none_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                          names=['min', 'max', 'num'], dtype=None, skip_footer=1)
            none_percentiles.append(getPercentile(none_values['max'], none_values['num'], 0.9))

            ftrace_values = np.genfromtxt(res_dir + 'ftrace_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                          names=['min', 'max', 'num'], dtype=None, skip_footer=1)
            ftrace_percentiles.append(getPercentile(ftrace_values['max'], ftrace_values['num'], 0.9))

            perf_values = np.genfromtxt(res_dir + 'perf_' + bytes + 'bytes_1process.hist', delimiter=',', skip_header=0,
                          names=['min', 'max', 'num'], dtype=None, skip_footer=1)
            perf_percentiles.append(getPercentile(perf_values['max'], perf_values['num'], 0.9))
            # rect_none = ax.bar(ind, none_values['totaltime'], width, color='b')

            rect_none = ax.bar(ind, none_values['num'], width, color='r')
            rect_lttng = ax.bar(ind + width, lttng_values['num'], width, color='b')
            rect_ftrace = ax.bar(ind + 2 * width, ftrace_values['num'], width, color='y')
            rect_perf = ax.bar(ind + 3 * width, perf_values['num'], width, color='m')

            # add some text for labels, title and axes ticks
            ax.set_ylabel('Time in ns')
            ax.set_title('Distribution of the latency per tracepoint for a payload of ' + bytes + ' bytes')
            # ax.set_xticks(ind + width)
            ax.set_xticks(ticks)
            # ax.set_xticklabels(lttng_values['max'])
            ax.set_xticklabels(ticks * 5)

            ax.legend((rect_none[0], rect_lttng[0], rect_ftrace[0], rect_perf[0]), ('None', 'LTTng', 'Ftrace', 'Perf'))

            fontP = FontProperties()
            fontP.set_size('small')
            plt.xlabel('Latency in ns')
            plt.ylabel('Frequency')
            rstyle(ax)
            # plt.xticks(range(80, 426, 5), fontsize=14)
            # plt.axis([16, 85, 0, int(args['loop'])])

            plt.tight_layout()
            # plt.show()
            imgname = 'pertp/hist_' + nprocess + 'proc_' + bytes + 'b'
            plt.savefig(imgname + '.png', dpi=100)

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



def is_outlier(points, thresh=500000000):
    if len(points.shape) == 1:
        points = points[:,None]
    median = np.median(points, axis=0)
    diff = np.sum((points - median)**2, axis=-1)
    diff = np.sqrt(diff)
    med_abs_deviation = np.median(diff)

    modified_z_score = 0.6745 * diff / med_abs_deviation

    return modified_z_score > thresh


"""
Get the nth percentile from values
"""
def getPercentile(values, density, n):
    population = 0
    count = 0

    if not n < 1:
        print('N must be smaller than 1')
        return -1

    if len(values) != len(density):
        print('Values and density should be the same size')
        return -1

    for i in density:
        population += i

    for i in range(1, len(density)):
        count += density[i]
        if count > population * n:
            return values[i]

    return density[len(density)]


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


