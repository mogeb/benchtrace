#!/usr/bin/python
# Copyright (c) PLUMgrid, Inc.
# Licensed under the Apache License, Version 2.0 (the "License")

from bcc import BPF
from time import sleep
from subprocess import call
import getopt
import sys
import ctypes as ct

class Event(ct.Structure):
    _fields_ = [
        ("ts", ct.c_ulonglong),
        ("payload", ct.c_int)
     ]

def print_event(cpu, data, size):
    event = ct.cast(data, ct.POINTER(Event)).contents
#    print(event.ts)

if __name__ == "__main__":
    b = BPF(src_file="ebpf-interface.c")
    b.attach_kprobe(event="tp_4b", fn_name="connect_tp")
    b["trace"].open_perf_buffer(print_event)
#    b.attach_kprobe(event="update_wall_time", fn_name="connect_tp")

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

#    while 1:
#        b.kprobe_poll()
