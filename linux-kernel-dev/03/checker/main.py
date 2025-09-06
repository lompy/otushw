#!/usr/bin/env python3

import unittest
import sys
from test_bin_search import BinarySearchTest
from test_bitmap import BitmapTest
from test_list import ListTest
from test_queue import QueueTest
from test_rb_tree import RBTreeTest

def main():
    suite = unittest.TestSuite()
    
    test_cases = [
        BinarySearchTest,
        BitmapTest,
        ListTest,
        QueueTest,
        RBTreeTest,
    ]
    
    for test_case in test_cases:
        suite.addTests(unittest.TestLoader().loadTestsFromTestCase(test_case))
    
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    return 0 if result.wasSuccessful() else 1

if __name__ == '__main__':
    sys.exit(main())
