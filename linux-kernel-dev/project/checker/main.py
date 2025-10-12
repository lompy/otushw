#!/usr/bin/env python3

import unittest
import sys

if __name__ == '__main__':
    # Load all tests from test_*.py files
    test_suite = unittest.defaultTestLoader.discover('.', pattern='test_*.py')
    
    # Run the tests
    result = unittest.TextTestRunner(verbosity=2).run(test_suite)
    
    # Exit with non-zero code if tests failed
    sys.exit(not result.wasSuccessful())
