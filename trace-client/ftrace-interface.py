import os
from subprocess import call

def start_tracing(tracename, tracepoints = None):
    call('echo 0 > /sys/kernel/debug/tracing/tracing_on', shell=True)
    call('echo > /sys/kernel/debug/tracing/trace', shell=True)
    call('echo 262144 > /sys/kernel/debug/tracing/buffer_size_kb', shell=True)
    call('echo 1 > /sys/kernel/debug/tracing/events/syscalls/sys_enter_getuid/enable', shell=True)
    call('echo 1 > /sys/kernel/debug/tracing/events/syscalls/sys_exit_getuid/enable', shell=True)
    call('echo nop > /sys/kernel/debug/tracing/current_tracer', shell=True)
    call('echo 1 > /sys/kernel/debug/tracing/tracing_on', shell=True)

def stop_tracing(tracename):
    call('echo 0 > /sys/kernel/debug/tracing/tracing_on', shell=True)