KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

format:
	clang-format -i ex_mutex.c ex_rwlock.c ex_semaphore.c ex_spinlock.c

check:
	sudo python3 checker/main.py
