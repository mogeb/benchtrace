from subprocess import call

def start_tracing(tracename, args = None, tracepoints = None):
    return


def run_command(cmd, args):
    print(cmd)
    call(cmd, shell=True)


def stop_tracing(tracename):
    return