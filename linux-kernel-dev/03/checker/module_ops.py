#!/usr/bin/env python3

import subprocess
import time

class ModuleOps:
    def __init__(self, module_name, parameters=None):
        self.module_name = module_name
        self.parameters = parameters
        self.parameter_path = f"/sys/module/{module_name}/parameters"
        self.attribute_path = f"/sys/kernel/{module_name}/"

    def load_module(self):
        """Load the kernel module with optional parameters"""
        try:
            cmd = ['sudo', 'insmod', f'{self.module_name}.ko']
            if self.parameters:
                # Add each parameter as param=value
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
            subprocess.run(['sudo', 'rmmod', f'{self.module_name}.ko'], check=True)
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

    def write_parameter(self, parameter, value):
        """Write a value to /sys/module/<module_name>/parameters/<parameter>"""
        try:
            with open(self.parameter_path + "/" + parameter, 'w') as f:
                f.write(str(value))
            return True
        except Exception as e:
            print(f"Failed to write to {parameter}: {e}")
            return False


    def write_attribute(self, attribute, value):
        """Write a value to /sys/kernel/<module_name>/<attribute>"""
        try:
            with open(self.attribute_path + attribute, 'w') as f:
                f.write(str(value))
            return True
        except Exception as e:
            print(f"Failed to write to {attribute}: {e}")
            return False

    def read_attribute(self, attribute):
        """Read a value from /sys/kernel/<module_name>/<attribute>"""
        try:
            with open(self.attribute_path + attribute, 'r') as f:
                return f.read().strip()
        except Exception as e:
            print(f"Failed to read from {attribute}: {e}")
            return None
