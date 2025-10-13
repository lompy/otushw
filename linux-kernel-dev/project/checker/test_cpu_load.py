#!/usr/bin/env python3

import unittest
import time
import re
from base_test import BaseModuleTest

class CPULoadTest(BaseModuleTest):
    module_name = "cpu_load"
    parameters = {
        "sample_interval": "100",
        "buffer_size": "64"
    }

    def test_device_creation(self):
        """Test device file is created"""
        import os
        self.assertTrue(os.path.exists("/dev/cpu_load"))

    def test_read_format(self):
        """Test device file content"""
        time.sleep(1)

        data = self.module_ops.read_cpu_load()
        self.assertIsNotNone(data)

        lines = data.strip().split('\n')
        self.assertGreater(len(lines), 0)

        cpu_pattern = re.compile(r'\s+(\d+):(\d{3})')

        for line in lines:
            if not line.strip():
                continue

            matches = cpu_pattern.findall(line)
            self.assertTrue(matches, f"No CPU load entries found in line: {line}")

            for cpu_id_str, load_str in matches:
                cpu_id = int(cpu_id_str)
                load = int(load_str)

                self.assertGreaterEqual(cpu_id, 0)
                self.assertLess(cpu_id, 100)
                self.assertGreaterEqual(load, 0)
                self.assertLessEqual(load, 100)

    def test_interval_change(self):
        """Test change the sampling interval"""
        new_interval = 500
        self.assertTrue(self.module_ops.set_interval(new_interval))

        time.sleep(1)
        data1 = self.module_ops.read_cpu_load()
        time.sleep(1)
        data2 = self.module_ops.read_cpu_load()

        self.assertNotEqual(data1, data2, "cpu load data should update with new interval")

if __name__ == '__main__':
    unittest.main()
