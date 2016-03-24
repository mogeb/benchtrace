from subprocess import call

jprobe_mod_file = '/home/mogeb/git/benchtrace/jprobe_bench/jprobe_bench.ko'
jprobe_mod_name = 'jprobe_bench'

def start_tracing(tracename, args, tracepoints = None):
    buf_size_kb = str(1 * int(args['buf_size_kb']))
    tp_size = args['tp_size']

    cmd = 'insmod ' + jprobe_mod_file
    print(cmd)
    call(cmd, shell=True)


def stop_tracing(tracename):
    cmd = 'rmmod ' + jprobe_mod_name
    print(cmd)
    call(cmd, shell=True)