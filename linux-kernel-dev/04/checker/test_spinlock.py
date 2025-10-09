#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class SpinlockTest(BaseModuleTest):
    module_name = "ex_spinlock"

    def test_spinlock(self):
        """Test spinlock"""
        self.assertInDmesg("ex_spinlock: loaded")
        self.assertInDmesg("ex_spinlock: unloaded, counter: 80")
