from subprocess import call

def start_tracing(tracename, args, tracepoints = None):
    pass


def run_command(cmd, args):
    command ='strace -q -t -f -o /dev/null %s' % (cmd)
    print(command)
    call(command, shell=True)


def stop_tracing(tracename):
    pass
