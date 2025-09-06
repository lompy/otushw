#!/usr/bin/env python3

import unittest
from module_ops import ModuleOps

class BaseModuleTest(unittest.TestCase):
    parameters = None
    def setUp(self):
        """Set up test fixtures before each test method"""
        self.module_ops = ModuleOps(self.module_name, self.parameters)
        self.module_ops.clear_dmesg()
        self.assertTrue(self.module_ops.load_module(), "Failed to load module")

    def tearDown(self):
        """Clean up after each test method"""
        self.assertTrue(self.module_ops.unload_module(), "Failed to unload module")
        self.module_ops.clear_dmesg()

    def assertInDmesg(self, text, msg=None):
        """Assert that text appears in dmesg output"""
        dmesg = self.module_ops.read_dmesg()
        self.assertIn(text, dmesg, msg)

    def assertNotInDmesg(self, text, msg=None):
        """Assert that text does not appear in dmesg output"""
        dmesg = self.module_ops.read_dmesg()
        self.assertNotIn(text, dmesg, msg)
