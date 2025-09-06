KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

format:
	clang-format -i ex_list.c ex_queue.c ex_bitmap.c ex_rb_tree.c ex_bin_search.c

check:
	sudo python3 checker/main.py
