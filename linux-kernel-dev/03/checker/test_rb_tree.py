#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class RBTreeTest(BaseModuleTest):
    module_name = "ex_rb_tree"

    def test_rb_tree_operations(self):
        test_strings = ["banana", "cherry", "apple"]
        
        for string in test_strings:
            self.assertTrue(
                self.module_ops.write_attribute("add_entry", string),
                f"Failed to add entry: {string}"
            )
        
        entries = self.module_ops.read_attribute("entries")
        self.assertIsNotNone(entries)
        entries_list = entries.strip().split('\n')
        self.assertEqual(entries_list, sorted(test_strings))
        
        self.assertTrue(
            self.module_ops.write_attribute("del_entry", "banana"),
            "Failed to delete entry"
        )
        
        entries = self.module_ops.read_attribute("entries")
        self.assertIsNotNone(entries)
        entries_list = entries.strip().split('\n')
        self.assertEqual(entries_list, ["apple", "cherry"])
