#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class QueueTest(BaseModuleTest):
    module_name = "ex_queue"

    def test_queue_operations(self):
        self.assertTrue(
            self.module_ops.write_attribute("put", "test1"),
            "Failed to put message"
        )
        
        message = self.module_ops.read_attribute("get")
        self.assertEqual(message, "test1")
        
        # queue capacity (CAP = 4)
        for i in range(4):
            self.assertTrue(
                self.module_ops.write_attribute("put", f"msg{i}"),
                f"Failed to put message {i}"
            )
        
        self.assertFalse(
            self.module_ops.write_attribute("put", "overflow"),
            "Should not accept message when queue is full"
        )
        
        for i in range(4):
            message = self.module_ops.read_attribute("get")
            self.assertEqual(message, f"msg{i}")
