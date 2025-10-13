#!/usr/bin/env python3

import subprocess
import time
import os
import fcntl
import struct

class ModuleOps:
    def __init__(self, module_name, parameters=None):
        self.module_name = module_name
        self.parameters = parameters
        self.parameter_path = f"/sys/module/{module_name}/parameters"
        self.device_path = f"/dev/{module_name}"

    def load_module(self):
        """Load the kernel module with optional parameters"""
        try:
            cmd = ['sudo', 'insmod', f'{self.module_name}.ko']
            if self.parameters:
                for param, value in self.parameters.items():
                    cmd.append(f'{param}={value}')
            
            subprocess.run(cmd, check=True)
            time.sleep(0.2)
            return True
        except subprocess.CalledProcessError as e:
            print(f"failed to load module: {e}")
            return False

    def unload_module(self):
        """Unload the kernel module"""
        try:
            subprocess.run(['sudo', 'rmmod', self.module_name], check=True)
            time.sleep(0.2)
            return True
        except subprocess.CalledProcessError as e:
            print(f"failed to unload module: {e}")
            return False

    def read_dmesg(self):
        """Read the kernel log messages filtered by module name"""
        try:
            result = subprocess.run(['sudo', 'dmesg'], capture_output=True, text=True, check=True)
            filtered_lines = [line for line in result.stdout.splitlines() if f"{self.module_name}: " in line]
            return '\n'.join(filtered_lines)
        except subprocess.CalledProcessError as e:
            print(f"Failed to read dmesg: {e}")
            return ""

    def clear_dmesg(self):
        """Clear the kernel log messages"""
        try:
            subprocess.run(['sudo', 'dmesg', '-c'], check=True)
            return True
        except subprocess.CalledProcessError as e:
            print(f"Failed to clear dmesg: {e}")
            return False

    def set_interval(self, interval_ms):
        """Set the sampling interval using ioctl"""
        CPU_LOAD_SET_INTERVAL = self._ioctl_code('c', 1, 'write')
        
        try:
            with open(self.device_path, 'rb') as f:
                fcntl.ioctl(f.fileno(), CPU_LOAD_SET_INTERVAL, struct.pack('i', interval_ms))
            return True
        except Exception as e:
            print(f"Failed to set interval: {e}")
            return False

    def read_cpu_load(self):
        """Read CPU load data from the device"""
        try:
            f = open(self.device_path, 'r')
            os.set_blocking(f.fileno(), False)
            return f.read().strip()
        except Exception as e:
            print(f"Failed to read CPU load: {e}")
            return None

    def _ioctl_code(self, type, nr, direction):
        """Generate ioctl command code"""
        _IOC_WRITE = 1
        _IOC_READ = 2
        
        if direction == 'read':
            direction = _IOC_READ
        elif direction == 'write':
            direction = _IOC_WRITE
        else:
            direction = _IOC_READ | _IOC_WRITE
            
        return direction << 30 | ord(type) << 8 | nr << 0 | 4 << 16  # 4 is sizeof(int)
