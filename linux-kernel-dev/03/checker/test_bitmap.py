#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class BitmapTest(BaseModuleTest):
    module_name = "ex_bitmap"
    parameters = {'nbits': 70}

    def test_bitmap_operations(self):
        self.assertInDmesg("allocated bitmap of 70 bits")
        self.assertInDmesg("zero: 00,00000000,00000000")
        self.assertInDmesg("set odd nbits: 2a,aaaaaaaa,aaaaaaaa")
        self.assertInDmesg("zero: 00,00000000,00000000")
