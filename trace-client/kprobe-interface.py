from subprocess import call

kprobe_mod_file = '/home/mogeb/git/benchtrace/kprobe_bench/kprobe_bench.ko'
kprobe_mod_name = 'kprobe_bench'

def start_tracing(tracename, args, tracepoints = None):
    buf_size_kb = str(1 * int(args['buf_size_kb']))
    tp_size = args['tp_size']

    cmd = 'insmod ' + kprobe_mod_file
    print(cmd)
    call(cmd, shell=True)


def stop_tracing(tracename):
    cmd = 'rmmod ' + kprobe_mod_name
    print(cmd)
    call(cmd, shell=True)
