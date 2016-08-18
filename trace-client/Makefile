DIR=v4

kernel_branches:
	cp $(DIR)/kernel/branches/*.hist .
	python3.4 trace-client.py -w cost-per-tp -t none,lttng,ftrace,perf,lttng-kprobe,systemtap,ebpf -p 1 -n 24999 -s 4 -b 8

kernel_misses:
	cp $(DIR)/kernel/misses/*.hist .
	python3.4 trace-client.py -w cost-per-tp -t none,lttng,ftrace,perf,lttng-kprobe,systemtap,ebpf -p 1 -n 24999 -s 4 -b 8

kernel_threads:
	cp $(DIR)/kernel/misses/*.hist .
	cp $(DIR)/kernel/threads/2/*.hist .
	cp $(DIR)/kernel/threads/4/*.hist .
	python3.4 trace-client.py -w cost-per-tp -t none,lttng,ftrace,perf,lttng-kprobe,systemtap,ebpf -p 1,2,4 -n 24999 -s 4 -b 8
