obj-m += kprobe_bench.o
benchmod-objs := kprobe_bench.o
ccflags-y += -I$(src)

all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) SUBDIRS=$(PWD) modules

clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean
