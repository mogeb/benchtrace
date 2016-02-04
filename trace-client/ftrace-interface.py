import os
from subprocess import call

tracing_dir = '/sys/kernel/debug/tracing/'

def start_tracing(tracename, tracepoints = None):
    call('echo 0 > ' + tracing_dir + 'tracing_on', shell=True)
    call('echo > ' + tracing_dir + 'trace', shell=True)
    call('echo 262144 > ' + tracing_dir + 'buffer_size_kb', shell=True)
    call('echo 1 > ' + tracing_dir + 'events/empty_tp/empty_ioctl_1b/enable', shell=True)
    call('echo nop > ' + tracing_dir + 'current_tracer', shell=True)
    call('echo 1 > ' + tracing_dir + 'tracing_on', shell=True)

def stop_tracing(tracename):
    call('echo 0 > ' + tracing_dir + 'tracing_on', shell=True)