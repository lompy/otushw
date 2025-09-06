#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class BinarySearchTest(BaseModuleTest):
    module_name = "ex_bin_search"
    parameters={'search_size': 450000}

    def test_search_size_parameter(self):
        self.assertInDmesg("found module ext4 with size 450000")
