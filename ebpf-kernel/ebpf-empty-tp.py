#!/usr/bin/python
# Copyright (c) PLUMgrid, Inc.
# Licensed under the Apache License, Version 2.0 (the "License")

from bcc import BPF
from time import sleep
from subprocess import call
import getopt
import sys

b = BPF(src_file="ebpf-interface.c")
b.attach_kprobe(event="tp_4b", fn_name="connect_tp")

# try:
#     print("Ctrl-C to stop")
#     sleep(99999999)
# except KeyboardInterrupt:
#     pass


if __name__ == "__main__":
    args = sys.argv
    try:
        optlist, args = getopt.getopt(args[1:], "p:n:")
    except getopt.GetoptError as err:
        error_opt(str(err))

    loops = 10
    threads = 1

    for opt, arg in optlist:
        if opt == "-n":
            loops = arg
        elif opt == "-p":
            threads = arg

    call(('/home/mogeb/git/benchtrace/all-calls/bin/allcalls -n %s -p %s') % (loops, threads), shell=True)

    #sleep(3)
    trace = b.get_table('trace')
#    for k, v in trace.items():
#        print("TS = %d, payload = %d" % (k.value, v.value))
