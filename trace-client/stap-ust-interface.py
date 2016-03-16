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
    return
    try:
        os.rmdir('kernel')
    except OSError:
        pass

    buf_size = int(args['buf_size_kb'])
    if buf_size >= 1024:
        buf_size = int(buf_size / 1024);
        buf_size_str = str(buf_size) + 'M'
    else:
        buf_size_str = str(buf_size) + 'k'

    tp_size = args['tp_size']
    num_subbuf = str(args['num_subbuf'])

    cmd = 'stap /home/mogeb/git/benchtrace/ustbench/ustbench.stp'
    print(cmd)
    call(cmd, shell=True)
    # call('sleep 3', shell=True)


"""
Disable tracing
"""
def stop_tracing(tracename):
    return
