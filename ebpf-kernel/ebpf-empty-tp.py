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

if __name__ == "__main__":
    b = BPF(src_file="ebpf-interface.c")
    b.attach_kprobe(event="tp_4b", fn_name="connect_tp")
    b["trace"].open_perf_buffer(print_event)

    args = sys.argv
    try:
        optlist, args = getopt.getopt(args[1:], "c:")
    except getopt.GetoptError as err:
        error_opt(str(err))

    for opt, arg in optlist:
        if opt == '-c':
            cmd = arg

    call(cmd, shell=True)
