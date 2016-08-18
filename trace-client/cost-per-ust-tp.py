import shutil
from pylab import *
from subprocess import call
from collections import defaultdict
from matplotlib.font_manager import FontProperties


tracers_colors = { 'none': 'r', 'lttng-ust': 'b', 'ftrace': 'y', 'lw-ust': 'm',
                   'stap-ust': 'black', 'printf': 'darksalmon',
                   'lttng-tracef': 'limegreen', 'extrae': 'teal' }


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
                ust_bin = '/home/mogeb/git/benchtrace/ustbench/bin/ustbench'

                # if tracer_name == 'stap-ust':
                #     cmd = '/home/mogeb/git/systemtap/stap /home/mogeb/git/benchtrace/systemtap/ustbench.stp'\
                #           ' -c "%s -n %s -p %s -t none"' % (ust_bin, loops, str(i))
                # else:
                cmd = '%s -n %s -p %s -t %s' % (ust_bin, loops, str(i), tracer_name)

                print(cmd)
                # call(cmd, shell=True)
                tracer.run_command(cmd, args)
                tracer.stop_tracing('session-test')
                shutil.copyfile('/tmp/out.csv', tracer_name + '_' + tp_size + 'bytes_' + buf_size_kb
                                + 'kbsubbuf_' + str(i) + '_process.hist')


def cleanup(args = None):
    return


def compile_results(args):
    compile_percentiles(args)
    compile_scatter_plot(args)
    # compile_scatter_plot_CPI(args)
    # compile_scatter_plot_nthreads(args)
    # compile_percentiles_nthreads(args)


def compile_percentiles(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    tp_sizes = args['tp_sizes']
    nprocesses = args['nprocesses']
    tracers = args['tracers']
    buf_sizes_kb = args['buf_sizes_kb']
    perc = 0.90

    for buf_size_kb in buf_sizes_kb:
        for nprocess in nprocesses:
            averages_file = open('averages', 'w')
            for tracer in tracers:
                percentiles = []
                for tp_size in tp_sizes:
                    fname = tracer + '_' + str(tp_size) + 'bytes_' + buf_size_kb\
                            + 'kbsubbuf_' + nprocess + '_process.hist'
                    with open(fname, 'r') as f:
                        legend = f.readline()
                    legend = legend.split(',')
                    values = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend,
                        dtype=None, invalid_raise=False)
                    percentiles.append(np.percentile(values['latency'], perc))
                    print('[%s] Average = %d' % (tracer, np.average(values['latency'])))
                    print('[%s] %dth percentile = %d' % (tracer, (perc * 100), np.percentile(values['latency'], perc)))
                    averages_file.write('[%s] Average = %d\n' % (tracer, np.average(values['latency'])))
                    averages_file.write('[%s] %dth percentile = %d\n' % (tracer, (perc * 100), np.percentile(values['latency'], perc)))
#                plt.plot(tp_sizes, percentiles, 'o-', label=tracer, color=tracers_colors[tracer])
            averages_file.close()
            plt.title(str(int(perc * 100)) + 'th percentiles for the cost of a tracepoint according to'
                                             'payload size')
            plt.xlabel('Payload size in bytes')
            plt.ylabel('Time in ns')
            fontP = FontProperties()
            fontP.set_size('small')

            # imgname = 'pertp/90th_' + nprocess + 'proc_' + str(args['buf_size_kb']) + 'subbuf_kb'
            plt.legend()
            plt.show()
            # plt.savefig(imgname + '.png', dpi=100)


def compile_scatter_plot(args):
    tp_size = args['tp_sizes'][0]
    tracers = args['tracers']
    buf_sizes_kb = args['buf_sizes_kb']
    nprocesses = args['nprocesses']
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    values = defaultdict(list)
    fname = res_dir + 'lttng-ust_' + tp_size + 'bytes_' + buf_sizes_kb[0] +\
            'kbsubbuf_' + nprocesses[0] + '_process.hist'
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


def compile_scatter_plot_nthreads(args):
    tp_size = args['tp_sizes'][0]
    tracers = args['tracers']
    buf_sizes_kb = args['buf_sizes_kb']
    nprocesses = args['nprocesses']
    tracer = 'lttng-ust'
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    values = defaultdict(list)
    fname = res_dir + 'lttng-ust_' + tp_size + 'bytes_' + buf_sizes_kb[0] + 'kbsubbuf_'\
            + nprocesses[0] + '_process.hist'
    # nthreads_colors = ['red', 'green', 'blue']
    nthreads_colors = {'1': 'blue', '2': 'coral', '4': 'red', '8': 'yellow', '16': 'darkred', '32': 'darkgray',
                       '64': 'm'}
    print(fname)
    with open(fname, 'r') as f:
        legend = f.readline()
    legend = legend.split(',')
    i = 0

    for nproc in nprocesses:
        fname = res_dir + tracer + '_' + tp_size + 'bytes_' + buf_sizes_kb[0] + 'kbsubbuf_'\
            + nproc + '_process.hist'
        values[nproc] = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None)

    fontP = FontProperties()
    fontP.set_size('small')
    for metric in legend:
        plt.subplot(2, 2, i)
        for nproc in nprocesses:
            if metric == 'latency':
                continue
            metric = metric.strip()
            plt.scatter(values[nproc][metric], values[nproc]['latency'], color=nthreads_colors[nproc], alpha=0.3, label=nproc)
        plt.title('Latency according to ' + metric)
        plt.xlabel(metric)
        plt.ylabel('Latency in ns')
        plt.legend(prop=fontP)
        i += 1
    plt.show()


def compile_percentiles_nthreads(args):
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    nprocesses = args['nprocesses']
    tracers = args['tracers']
    buf_sizes_kb = args['buf_sizes_kb']
    tp_size = 4
    percs = [0.5, 0.9]
    tracers = args['tracers']
    buf_size_kb = buf_sizes_kb[0]
    percentiles = defaultdict(list)


    perc = 0.9
    for tracer in tracers:
        for nproc in nprocesses:
            fname = tracer + '_' + str(tp_size) + 'bytes_' + buf_size_kb\
                                + 'kbsubbuf_' + nproc + '_process.hist'
            with open(fname, 'r') as f:
                legend = f.readline()
            legend = legend.split(',')
            values = np.genfromtxt(fname, delimiter=',', skip_header=1, names=legend, dtype=None, invalid_raise=False)
            percentiles[tracer].append(np.average(values['latency']))

        plt.plot(nprocesses, percentiles[tracer], 'o-', label=tracer)
        plt.title('[' + tracer + '] Average of tracepoint latency according to the number of threads')
        plt.xlabel('Number of threads')
        plt.ylabel('Latency in ns')
        fontP = FontProperties()
        fontP.set_size('small')
        # plt.axis([0, 9, 0, 900])
        plt.legend(prop=fontP, loc='upper left')
        plt.show()
    return


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

            # imgname = 'pertp/' + str(int(perc * 100)) + 'th_' + nprocess + 'proc_' + str(args['buf_size_kb']) + 'subbuf_kb'
            plt.axis([0, 9, 0, 900])
            plt.legend(prop=fontP, loc='upper left')
            plt.show()
            # plt.savefig(imgname + '.png', dpi=100)


def compile_scatter_plot_CPI(args):
    tp_size = args['tp_sizes'][0]
    tracers = args['tracers']
    res_dir = '/home/mogeb/git/benchtrace/trace-client/'
    buf_sizes_kb = args['buf_sizes_kb']
    nprocesses = args['nprocesses']
    values = defaultdict(list)
    cpi = defaultdict(list)
    ipc = defaultdict(list)
    handles = defaultdict(list)

    fname = tracers[0] + '_' + str(tp_size) + 'bytes_' + buf_sizes_kb[0]\
                            + 'kbsubbuf_' + nprocesses[0] + '_process.hist'
    with open(fname, 'r') as f:
        legend = f.readline()
    legend = legend.split(',')
    i = 0
    for tracer in tracers:
        fname = res_dir + tracer + '_' + str(tp_size) + 'bytes_' + buf_sizes_kb[0]\
                            + 'kbsubbuf_' + nprocesses[0] + '_process.hist'
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
        plt.scatter(cpi[tracer], values[tracer]['latency'], color=tracers_colors[tracer], alpha=0.3,
                    label=tracer)
    plt.legend(prop=fontP)

    plt.subplot(2, 1, 1)
    plt.title('Latency according to IPC')
    plt.xlabel('IPC')
    plt.ylabel('Latency in ns')
    plt.legend(prop=fontP)
    for tracer in tracers:
        plt.scatter(ipc[tracer], values[tracer]['latency'], color=tracers_colors[tracer], alpha=0.3,
                    label=tracer)

    plt.legend(prop=fontP)
    plt.show()
