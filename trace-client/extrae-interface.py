from subprocess import call
import os


def start_tracing(tracename, args, tracepoints = None):
    pass


def run_command(cmd, args):
    call('EXTRAE_CONFIG_FILE=/home/mogeb/paraver/extrae/share/example/PTHREAD/extrae.xml /home/mogeb/paraver/extrae/share/example/PTHREAD/my-example', shell=True)


def stop_tracing(tracename):
    call('lttng stop', shell=True)
    call('lttng destroy ' + tracename, shell=True)
