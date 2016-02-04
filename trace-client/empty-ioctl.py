from subprocess import call

def init(args = None):
    return

def do_work(tracer, args = None):
    #call("perf stat -e 'syscalls:*' /home/mogeb/git/benchtrace/bench-client/client -t "
    for i in [1, 2, 3, 4, 8, 16, 32, 64]:
        if tracer == 'perf':
            call("perf stat -e 'empty_tp:*' /home/mogeb/git/benchtrace/bench-client/allcalls -t "
                 + tracer + " -n 80000000 -p " + str(i) + " -o " + tracer + ".out", shell=True)
        else:
            call("/home/mogeb/git/benchtrace/bench-client/allcalls -t "
                 + tracer + " -n 80000000 -p " + str(i) + " -o " + tracer + ".out", shell=True)

def cleanup(args = None):
    return

def compile_results():
    return