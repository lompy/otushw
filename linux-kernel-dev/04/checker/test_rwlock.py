#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class RWLockTest(BaseModuleTest):
    module_name = "ex_rwlock"

    def test_rwlock(self):
        """Test rwlock"""
        self.assertInDmesg("ex_rwlock: loaded")
        self.assertInDmesg("ex_rwlock: unloaded, value:")
