import sys
import os
from subprocess import call


def start_tracing(tracename, args, tracepoints = None):
    pass


def run_command(cmd, args):
    call('python2 /home/mogeb/git/benchtrace/ebpf-kernel/ebpf-empty-tp.py -c "%s"' %\
        (cmd), shell=True)


def stop_tracing(tracename):
    pass
