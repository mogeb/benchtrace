from subprocess import call

tracing_dir = '/sys/kernel/debug/tracing/'


def start_tracing(tracename, args, tracepoints = None):
    print('echo 0 > ' + tracing_dir + 'tracing_on')
    call('echo 0 > ' + tracing_dir + 'tracing_on', shell=True)

    print('echo > ' + tracing_dir + 'trace')
    call('echo > ' + tracing_dir + 'trace', shell=True)

    print('echo ' + args['buf_size_kb'] + ' > ' + tracing_dir + 'buffer_size_kb')
    call('echo ' + args['buf_size_kb'] + ' > ' + tracing_dir + 'buffer_size_kb', shell=True)

    print('echo 1 > ' + tracing_dir + 'events/empty_tp/empty_ioctl_1b/enable')
    call('echo 1 > ' + tracing_dir + 'events/empty_tp/empty_ioctl_1b/enable', shell=True)

    print('echo nop > ' + tracing_dir + 'current_tracer')
    call('echo nop > ' + tracing_dir + 'current_tracer', shell=True)

    print('echo 1 > ' + tracing_dir + 'tracing_on')
    call('echo 1 > ' + tracing_dir + 'tracing_on', shell=True)


def stop_tracing(tracename):
    call('echo 0 > ' + tracing_dir + 'tracing_on', shell=True)
    call('head ' + tracing_dir + 'trace', shell=True)