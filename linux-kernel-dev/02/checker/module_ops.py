import sys

class ModuleOps:
    def __init__(self, idx_file, ch_val_file, my_str_file):
        self.idx_file = idx_file
        self.ch_val_file = ch_val_file
        self.my_str_file = my_str_file

    def write_idx(self, value:str):
        try:
            self.idx_file.seek(0)
            self.idx_file.write(value)
            self.idx_file.flush()
        except Exception as e:
            return str(e)

    def write_ch_val(self, value:str):
        try:
            val_str = str(value)
            self.ch_val_file.seek(0)
            self.ch_val_file.write(val_str[0])
            self.ch_val_file.flush()
        except Exception as e:
            return str(e)
            
    def read_idx(self):
        try:
            self.idx_file.seek(0)
            return int(self.idx_file.read().strip())
        except Exception as e:
            return str(e)

    def read_ch_val(self):
        try:
            self.ch_val_file.seek(0)
            ch = self.ch_val_file.read(1).strip()
            return ch
        except Exception as e:
            return str(e)

    def read_my_str(self):
        self.my_str_file.seek(0)
        my_str = self.my_str_file.read()
        return my_str
