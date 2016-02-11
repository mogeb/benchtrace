import sys
import os
from subprocess import call

try:
    import lttng
except ImportError:
    sys.path.append("/usr/local/lib/python%d.%d/site-packages" %
                    (sys.version_info.major, sys.version_info.minor))
    import lttng


"""
Enable tracing
"""
def start_tracing(tracename, args, tracepoints = None):
    try:
        os.rmdir('kernel')
    except OSError:
        pass

    buf_size = args['buf_size_kb'] * 1024
    call('lttng create ' + tracename + ' -o /home/mogeb/git/benchtrace/trace-client', shell=True)
    call('lttng enable-channel chan0 --subbuf-size ' + buf_size + ' -k', shell=True)
    call('lttng enable-event -k -c chan0 empty_ioctl_1b', shell=True)
    call('lttng start', shell=True)


"""
Disable tracing
"""
def stop_tracing(tracename):
    call('lttng stop', shell=True)
    call('lttng destroy ' + tracename, shell=True)


"""
Enable tracing using Python binding for LTTng, not used currently.
"""
def start_tracing_binding(tracename, tracepoints = None):
    os.remove('lttng.out')
    lttng.create(tracename, "/home/mogeb/git/benchtrace/trace-client")
    domain = lttng.Domain()
    domain.type = lttng.DOMAIN_KERNEL
    handle = lttng.Handle(tracename, domain)
    lttng.channel_set_default_attr
    lttng.list_tracepoints(handle)
    event = lttng.Event()
    event.name = 'empty_ioctl_1b'
    #event.name = 'getuid'
    #event.type = lttng.EVENT_SYSCALL
    lttng.enable_event(handle, event, None)
    lttng.start(tracename)


"""
Enable tracing using Python binding for LTTng, not used currently.
"""
def stop_tracing_binding(tracename):
    lttng.stop(tracename)
    lttng.destroy(tracename)
