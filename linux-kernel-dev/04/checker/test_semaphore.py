#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class SemaphoreTest(BaseModuleTest):
    module_name = "ex_semaphore"

    def test_semaphore(self):
        """Test semaphore"""
        self.assertInDmesg("producer 0: produced item 9")
        self.assertInDmesg("consumed item 9")
        self.assertInDmesg("producer 1: produced item 1009")
        self.assertInDmesg("consumed item 1009")
        self.assertInDmesg("producer 0: completed")
        self.assertInDmesg("producer 1: completed")
        self.assertInDmesg("consumer 0: completed")
        self.assertInDmesg("consumer 1: completed")
        self.assertInDmesg("consumer 2: completed")
