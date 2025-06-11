KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

format:
	clang-format -i hw_hello.c
