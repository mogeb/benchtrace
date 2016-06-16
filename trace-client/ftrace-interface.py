from subprocess import call

tracing_dir = '/sys/kernel/debug/tracing/'


def start_tracing(tracename, args, tracepoints = None):
    buf_size_kb = str(int(args['num_subbuf']) * int(args['buf_size_kb']))
    tp_size = args['tp_size']

    print('echo 0 > ' + tracing_dir + 'tracing_on')
    call('echo 0 > ' + tracing_dir + 'tracing_on', shell=True)

    print('echo > ' + tracing_dir + 'trace')
    call('echo > ' + tracing_dir + 'trace', shell=True)

    print('echo global > ' + tracing_dir + 'trace_clock')
    call('echo global > ' + tracing_dir + 'trace_clock', shell=True)

    print('echo ' + buf_size_kb + ' > ' + tracing_dir + 'buffer_size_kb')
    call('echo ' + buf_size_kb + ' > ' + tracing_dir + 'buffer_size_kb', shell=True)

    print('echo 1 > ' + tracing_dir + 'events/empty_tp/empty_ioctl_' + tp_size + 'b/enable')
    call('echo 1 > ' + tracing_dir + 'events/empty_tp/empty_ioctl_' + tp_size + 'b/enable', shell=True)

    print('echo nop > ' + tracing_dir + 'current_tracer')
    call('echo nop > ' + tracing_dir + 'current_tracer', shell=True)

    print('echo 1 > ' + tracing_dir + 'tracing_on')
    call('echo 1 > ' + tracing_dir + 'tracing_on', shell=True)


def run_command(cmd, args):
    print(cmd)
    call(cmd, shell=True)


def stop_tracing(tracename):
    call('echo 0 > ' + tracing_dir + 'tracing_on', shell=True)
    call('head ' + tracing_dir + 'trace', shell=True)

    print('find ' + tracing_dir + 'events/empty_tp/ -name enable -exec bash -c "echo 0 > {}" \;')
    call('find ' + tracing_dir + 'events/empty_tp/ -name enable -exec bash -c "echo 0 > {}" \;', shell=True)
