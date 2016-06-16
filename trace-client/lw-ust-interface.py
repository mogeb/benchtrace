from subprocess import call


def start_tracing(tracename, args, tracepoints = None):
    pass


def run_command(cmd, args):
    print(cmd)
    call(cmd, shell=True)


def stop_tracing(tracename):
    return
