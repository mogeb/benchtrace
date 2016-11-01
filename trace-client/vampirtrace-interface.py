from subprocess import call
import os


def start_tracing(tracename, args, tracepoints = None):
    pass


def run_command(cmd, args):
    call('VT_ONOFF_CHECK_STACK_BALANCE=no /home/mogeb/git/benchtrace/vampirbench/bin/vampirbench -n 24999 -p 1', shell=True)
    # call('EXTRAE_CONFIG_FILE=/home/mogeb/git/benchtrace/extrae/extrae.xml /home/mogeb/git/benchtrace/extrae/bin/extrae-bench', shell=True)


def stop_tracing(tracename):
    pass
