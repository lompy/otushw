import os
import sys

from module_ops import ModuleOps
from tests import Tests

class Checker:
    def __init__(self, module_name):
        self.module_name = module_name
        self.idx_path    = f"/sys/module/{module_name}/parameters/idx"
        self.ch_val_path = f"/sys/module/{module_name}/parameters/ch_val"
        self.my_str_path = f"/sys/module/{module_name}/parameters/my_str"
        self.idx_file = None
        self.ch_val_file = None
        self.my_str_file = None
        self.open_sysfs_files()

        self.ops = ModuleOps(self.idx_file, self.ch_val_file, self.my_str_file)
        self.tests = Tests(self.ops)

    def open_sysfs_files(self, mode='r+'):
        try:
            self.idx_file = open(self.idx_path, mode)
            self.ch_val_file = open(self.ch_val_path, mode)
            self.my_str_file = open(self.my_str_path, "r")
        except Exception as e:
            print(f"Ошибка при открытии файлов sysfs: {e}")
            sys.exit(1)

    def check_idx(self):
        return self.tests.idx_tests()

    def check_ch_val(self):
        return self.tests.ch_val_tests()

    def check_my_str(self):
        self.open_sysfs_files()
        return self.tests.my_str_tests()
