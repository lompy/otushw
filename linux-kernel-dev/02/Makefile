KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += hw_hello.o

default:
	make -C $(KERNEL_DIR) M=$(PWD) modules

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

format:
	clang-format -i hw_hello.c
