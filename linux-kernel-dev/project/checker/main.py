#!/usr/bin/env python3

import unittest
import sys
from test_cpu_load import CPULoadTest

def main():
    suite = unittest.TestSuite()

    test_cases = [
        CPULoadTest,
    ]

    for test_case in test_cases:
        suite.addTests(unittest.TestLoader().loadTestsFromTestCase(test_case))

    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    return 0 if result.wasSuccessful() else 1

if __name__ == '__main__':
    sys.exit(main())
