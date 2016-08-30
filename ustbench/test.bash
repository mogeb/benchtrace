#!/bin/bash

lttng create session-test -o .
lttng enable-channel chan0 --subbuf-size 8k --num-subbuf 2 -u
lttng enable-event -u -c chan0 ustbench:bench_tp_4b
lttng start
/home/mogeb/git/benchtrace/ustbench/bin/ustbench -n 100 -p 1 -t lttng-ust
lttng stop
lttng destroy
