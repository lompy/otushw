from module_ops import ModuleOps

class Executor:
    def __init__(self, ops):
        self.ops = ops

    def idx_exec_test(self, case):
        self.ops.write_idx(case['write'])
        value = self.ops.read_idx()
        print(f"idx_test: записано {case['write']}, прочитано {value}, ожидалось {case['expected']}")
        return str(value) == case['expected']

    def ch_val_exec_test(self, case):
        self.ops.write_ch_val(case['write'])
        value = self.ops.read_ch_val()
        print(f"ch_val_test: записано {case['write']}, прочитано {value}, ожидалось {case['expected']}")
        return str(value) == case['expected']

    def my_str_exec_test(self, case):
        _str = case['write']
        for idx in range (0, 13):
            self.ops.write_idx(str(idx))
            self.ops.write_ch_val(" ")
        for idx in range (0, len(_str)):
            self.ops.write_idx(str(idx))
            self.ops.write_ch_val(_str[idx])
        my_str = self.ops.read_my_str()
        print(f"my_str_test: записано {case['write']}, прочитано {my_str}, ожидалось {case['expected']}")
        return my_str.strip() == case['expected']
