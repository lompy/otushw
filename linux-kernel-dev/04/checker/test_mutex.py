#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class MutexTest(BaseModuleTest):
    module_name = "ex_mutex"

    def test_mutex_counter(self):
        """Test mutex counter"""
        self.assertInDmesg("ex_mutex: loaded")
        self.assertInDmesg("ex_mutex: unloaded, counter: 15")
