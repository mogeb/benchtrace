import os
from subprocess import call


def start_tracing(tracename, args, tracepoints = None):
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

    num_subbuf = str(args['num_subbuf'])

    cmd = 'lttng create --snapshot ' + tracename +\
          ' -o /home/mogeb/git/benchtrace/trace-client'
    print(cmd)
    call(cmd, shell=True)

    cmd = 'lttng enable-channel chan0 --subbuf-size ' + buf_size_str + ' --num-subbuf '\
          + num_subbuf + ' -u'
    print(cmd)
    call(cmd, shell=True)

    cmd = 'lttng enable-event -u -c chan0 lttng_ust_tracef:event'
    print(cmd)
    call(cmd, shell=True)

    cmd = 'lttng start'
    print(cmd)
    call(cmd, shell=True)


def run_command(cmd, args):
    print(cmd)
    call(cmd, shell=True)


def stop_tracing(tracename):
    call('lttng stop', shell=True)
    call('lttng destroy ' + tracename, shell=True)
