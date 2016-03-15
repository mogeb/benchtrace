import shutil
from pylab import *
from subprocess import call
from collections import defaultdict
from matplotlib.font_manager import FontProperties


tracers_colors = { 'none': 'r', 'lttng-ust': 'b', 'ftrace': 'y', 'perf': 'm',
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
                cmd = '/home/mogeb/git/benchtrace/ustbench/ustbench ' + " -n " + loops + " -p " + str(i)
                print(cmd)
                call(cmd, shell=True)
                tracer.stop_tracing('session-test')
                shutil.copyfile('/tmp/out.csv', tracer_name + '_' + tp_size + 'bytes_' + buf_size_kb
                                + 'kbsubbuf_' + i + '_process.hist')


def cleanup(args = None):
    return


def compile_results(args):
    compile_scatter_plot(args)


def compile_scatter_plot(args):
    tp_size = args['tp_sizes'][0]
    tracers = args['tracers']
    buf_sizes_kb = args['buf_sizes_kb']
    nprocesses = args['nprocesses']
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    values = defaultdict(list)
    fname = res_dir + 'lttng-ust_' + tp_size + 'bytes_' + buf_sizes_kb[0] + 'kbsubbuf_'\
            + nprocesses[0] + '_process.hist'
    print(fname)
    with open(fname, 'r') as f:
        legend = f.readline()
    legend = legend.split(',')
    i = 0
    for tracer in tracers:
        fname = res_dir + tracer + '_' + tp_size + 'bytes_' + buf_sizes_kb[0] + 'kbsubbuf_'\
            + nprocesses[0] + '_process.hist'
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