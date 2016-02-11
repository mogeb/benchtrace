import getopt
import sys
from subprocess import call


def error_opt(msg):
    print("Option error: " + msg)


def main(args):
    call(["rm", "-rf", "/home/mogeb/git/benchtrace/trace-client/kernel"])
    longopts = ["tracer=", "workload="]
    try:
        optlist, args = getopt.getopt(args[1:], "t:w:", longopts)
    except getopt.GetoptError as err:
        error_opt(str(err))

    for opt, arg in optlist:
        if opt == "--tracer" or opt == "-t":
            tracer_args = arg.split(",")
        elif opt == "--workload" or opt == "-w":
            workload_args = arg.split(",")

    for workload_arg in workload_args:
        try:
            workload = __import__(workload_arg)
        except ImportError as err:
            print('Import error: ' + str(err))

        for tracer_arg in tracer_args:
            try:
                tracer = __import__(tracer_arg + "-interface")
            except ImportError as err:
                print('Import error: ' + str(err))

            args['buf_size_kb'] = 256
            print()
            print('----')
            print('Starting for tracer "' + tracer_arg + '"')
            print()
            workload.init()
            """
            Function do_work() should take care of enabling/disabling tracing
            """
            workload.do_work(tracer, args, tracer_arg)
            workload.cleanup()

        workload.compile_results()


if __name__ == "__main__":
    main(sys.argv)
