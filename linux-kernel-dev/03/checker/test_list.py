#!/usr/bin/env python3

import unittest
from base_test import BaseModuleTest

class ListTest(BaseModuleTest):
    module_name = "ex_list"

    def test_list_operations(self):
        """Test adding entries to the list"""
        test_strings = ["test1", "test2", "test3"]
        
        for string in test_strings:
            self.assertTrue(
                self.module_ops.write_attribute("add_entry", string),
                f"Failed to add entry: {string}"
            )
        
        entries = self.module_ops.read_attribute("entries")
        self.assertIsNotNone(entries)
        
        for string in test_strings:
            self.assertIn(string, entries)

        self.assertTrue(
            self.module_ops.write_attribute("del_entry", "test2"),
            "Failed to delete entry"
        )

        entries = self.module_ops.read_attribute("entries")
        self.assertIsNotNone(entries)
        self.assertIn("test1", entries)
        self.assertNotIn("test2", entries)
        self.assertIn("test3", entries)
