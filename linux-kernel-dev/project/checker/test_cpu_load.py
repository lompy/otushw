#!/usr/bin/env python3

import unittest
import time
from base_test import BaseModuleTest

class CpuLoadTest(BaseModuleTest):
    module_name = "cpu_load"
    parameters = {
        "sample_interval": "1000",
        "buffer_size": "60"
    }

    def test_device_creation(self):
        """Test if the device file is created"""
        import os
        self.assertTrue(os.path.exists("/dev/cpu_load"))

    def test_read_format(self):
        """Test if the output format is correct"""
        # Wait for a few samples to be collected
        time.sleep(2)
        
        data = self.module_ops.read_cpu_load()
        self.assertIsNotNone(data)
        
        lines = data.strip().split('\n')
        self.assertGreater(len(lines), 0)
        
        # Each line should be: cpu_id,total_ticks,idle_ticks
        for line in lines:
            parts = line.split(',')
            self.assertEqual(len(parts), 3)
            
            # Verify types
            cpu_id = int(parts[0])
            total_ticks = int(parts[1])
            idle_ticks = int(parts[2])
            
            self.assertGreaterEqual(cpu_id, 0)
            self.assertGreaterEqual(total_ticks, 0)
            self.assertGreaterEqual(idle_ticks, 0)
            self.assertLessEqual(idle_ticks, total_ticks)

    def test_interval_change(self):
        """Test if we can change the sampling interval"""
        # Set new interval
        new_interval = 500
        self.assertTrue(self.module_ops.set_interval(new_interval))
        
        # Wait and verify we get new samples
        time.sleep(1)
        data1 = self.module_ops.read_cpu_load()
        time.sleep(0.5)
        data2 = self.module_ops.read_cpu_load()
        
        self.assertNotEqual(data1, data2, "CPU load data should update with new interval")

if __name__ == '__main__':
    unittest.main()
