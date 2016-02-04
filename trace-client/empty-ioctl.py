from subprocess import call

def init(args = None):
    print("Empty ioctl workload init")
    return

def do_work(tracer, args = None):
    #call("perf stat -e 'syscalls:*' /home/mogeb/git/benchtrace/bench-client/client -t "
    for i in [1, 2, 4, 8, 16, 32, 64]:
        call("perf stat -e 'syscalls:*' /home/mogeb/git/benchtrace/bench-client/client -t "
        #call("/home/mogeb/git/benchtrace/bench-client/client -t "
             + tracer + " -n 80000000 -p " + str(i) + " -o " + tracer + ".out", shell=True)

def cleanup(args = None):
    print("Empty ioctl workload cleanup")
    return