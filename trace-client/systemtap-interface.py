from subprocess import call

def start_tracing(tracename, args, tracepoints = None):
    pass


def run_command(cmd, args):
    command ='/home/mogeb/git/systemtap/stap -k ' \
             '/home/mogeb/git/benchtrace/systemtap/do_tp.stp -c "%s"' % (cmd)
    print(command)
    call(command, shell=True)


def stop_tracing(tracename):
    pass
