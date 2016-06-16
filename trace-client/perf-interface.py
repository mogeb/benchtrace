from subprocess import call


def start_tracing(tracename, args, tracepoints = None):
    return


def run_command(cmd, args):
    tp_size_str = str(args['tp_size'])

    perf_bin = '/home/mogeb/src/linux-4.5/tools/perf/perf'
    call(perf_bin + " record -o /dev/null -e 'empty_tp:empty_ioctl_" +
         tp_size_str + "b' " + cmd, shell=True)


def stop_tracing(tracename):
    return
