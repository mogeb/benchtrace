from subprocess import call

def init(args = None):
    print("Single call workload init")
    return

def do_work(tracer, tracer_name, args = None):
    for i in [1, 2, 4, 8]:
        tracer.start_tracing('session-test')
        if tracer_name == 'perf':
            call("perf stat -e 'empty_tp:*' /home/mogeb/git/benchtrace/single-call/singlecall -t "
                 + tracer_name + " -n 8000000 -p " + str(i) + " -o " + tracer_name + ".out", shell=True)
        else:
            call("/home/mogeb/git/benchtrace/single-call/singlecall -t "
                 + tracer_name + " -n 8000000 -p " + str(i) + " -o sc_" + tracer_name + ".out", shell=True)
        tracer.stop_tracing('session-test')

def cleanup(args = None):
    print("Single call workload cleanup")
    return

def compile_results():
    return