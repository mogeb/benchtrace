import sys
import os
from subprocess import call

try:
    import lttng
except ImportError:
    sys.path.append("/usr/local/lib/python%d.%d/site-packages" %
                    (sys.version_info.major, sys.version_info.minor))
    import lttng


def start_tracing(tracename, args, tracepoints = None):
    pass


def run_command(cmd, args):
    command = '/home/mogeb/git/systemtap/stap ' \
              '/home/mogeb/git/benchtrace/systemtap/ustbench.stp -c "%s"' % (cmd)
    print(command)
    call(command, shell=True)


def stop_tracing(tracename):
    pass
