obj-m += dummy_tracer.o
ccflags-y += -I$(src) -I$(src)/../utils

all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) SUBDIRS=$(PWD) modules

install:
	install dummy_tracer.ko /lib/modules/`uname -r`/extra/
	depmod -a

clean:
	make -C /lib/modules/`uname -r`/build M=$(PWD) clean
