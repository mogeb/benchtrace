from subprocess import call

def init(args = None):
    print("Single call workload init")
    return

def do_work(tracer, args = None):
    #call("perf stat -e 'syscalls:*' /home/mogeb/git/benchtrace/single-call/singlecall -t "
    for i in [1, 2, 4, 8, 16, 32, 64]:
        #call("perf stat -e 'syscalls:*' /home/mogeb/git/benchtrace/single-call/singlecall -t "
        call("/home/mogeb/git/benchtrace/single-call/singlecall -t "
             + tracer + " -n 80000000 -p " + str(i) + " -o sc_" + tracer + ".out", shell=True)

def cleanup(args = None):
    print("Single call workload cleanup")
    return

def compile_results():
    return