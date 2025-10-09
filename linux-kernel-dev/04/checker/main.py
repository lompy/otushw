#!/usr/bin/env python3

import unittest
import sys
from test_mutex import MutexTest
from test_rwlock import RWLockTest
from test_semaphore import SemaphoreTest
from test_spinlock import SpinlockTest

def main():
    suite = unittest.TestSuite()
    
    test_cases = [
        MutexTest,
        RWLockTest,
        SemaphoreTest,
        SpinlockTest,
    ]
    
    for test_case in test_cases:
        suite.addTests(unittest.TestLoader().loadTestsFromTestCase(test_case))
    
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    return 0 if result.wasSuccessful() else 1

if __name__ == '__main__':
    sys.exit(main())
