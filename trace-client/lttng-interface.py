import sys
try:
    import lttng
except ImportError:
    sys.path.append("/usr/local/lib/python%d.%d/site-packages" %
                    (sys.version_info.major, sys.version_info.minor))
    import lttng


def start_tracing(tracename, tracepoints = None):
    print("LTTng start tracing")
    lttng.create(tracename, "/home/mogeb/git/benchtrace/trace-client")
    domain = lttng.Domain()
    domain.type = lttng.DOMAIN_KERNEL
    handle = lttng.Handle(tracename, domain)
    event = lttng.Event()
    event.name = 'empty_ioctl_1b'
    #event.name = 'getuid'
    #event.type = lttng.EVENT_SYSCALL
    lttng.enable_event(handle, event, None)
    lttng.start(tracename)

def stop_tracing(tracename):
    print("LTTng stop tracing")
    lttng.stop(tracename)
    lttng.destroy(tracename)
