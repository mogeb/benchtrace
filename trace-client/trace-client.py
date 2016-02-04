import getopt
import sys
from subprocess import call

def error_opt(msg):
    print("Option error: " + msg)

def main(args):
    #call(["rm", "-rf", "/home/mogeb/PycharmProjects/trace-client/kernel"])
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
            print("Import error: " + str(err))

        for tracer_arg in tracer_args:
            try:
                tracer = __import__(tracer_arg + "-interface")
            except ImportError as err:
                print("Import error: " + str(err))

            workload.init()
            tracer.start_tracing("session-test")
            workload.do_work(tracer_arg)
            tracer.stop_tracing("session-test")
            workload.cleanup()

if __name__ == "__main__":
    main(sys.argv)
