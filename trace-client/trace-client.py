import getopt
import sys
from subprocess import call


def error_opt(msg):
    print("Option error: " + msg)


def main(args):
    longopts = ["tracer=", "workload=", "size=", "process=", "loop=", "subbuf-size", "num-subbuf="]
    try:
        optlist, args = getopt.getopt(args[1:], "t:w:s:p:n:zb:", longopts)
    except getopt.GetoptError as err:
        error_opt(str(err))

    """
    Default values
    """
    loop = 100
    # buf_size_kb = 524288
    buf_size_kb = 262144
    do_work = False
    num_sub_buf_kb = 4
    buf_sizes_kb = [131072]
    tp_sizes = [4]

    """
    Parse arguments
    """
    for opt, arg in optlist:
        if opt == "--tracer" or opt == "-t":
            tracer_args = arg.split(",")
        elif opt == "--workload" or opt == "-w":
            workload_args = arg.split(",")
        elif opt == "--payload-size" or opt == "-s":
            tp_sizes = arg.split(",")
        elif opt == "--process" or opt == "-p":
            nprocesses = arg.split(",")
        elif opt == "--loop" or opt == "-n":
            loop = arg
        elif opt == "-z":
            do_work = True
        elif opt == "--subbuf-size" or opt == "-b":
            buf_sizes_kb = arg.split(",")
        elif opt == "--num-subbuf":
            num_sub_buf_kb = arg

    for i in range(0, len(buf_sizes_kb)):
        if int(buf_sizes_kb[i]) > 262144:
            print("OVERRIDING BUFFER SIZE TO 262144")
            buf_size_kb[buf_sizes_kb] = str(262144)


    for workload_arg in workload_args:
        try:
            workload = __import__(workload_arg)
        except ImportError as err:
            print('Import error: ' + str(err))

        args = { 'buf_sizes_kb' : buf_sizes_kb, 'tp_sizes': tp_sizes,
                 'nprocesses' : nprocesses, 'loop' : loop, 'tracers': tracer_args,
                 'num_subbuf': num_sub_buf_kb}
        if do_work:
            for tracer_arg in tracer_args:
                try:
                    tracer = __import__(tracer_arg + "-interface")
                except ImportError as err:
                    print('Import error: ' + str(err))

                print()
                print('----')
                print('Starting for tracer "' + tracer_arg + '"')
                print()
                workload.init()
                """
                Function do_work() should take care of enabling/disabling tracing
                """
                workload.do_work(tracer, tracer_arg, args)
                workload.cleanup()

        workload.compile_results(args)


if __name__ == "__main__":
    main(sys.argv)
