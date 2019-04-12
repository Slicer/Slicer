from __future__ import print_function
import slicer

import unittest

class PythonManagerTests(unittest.TestCase):

    def setUp(self):
        pass

    def test_toPythonStringLiteral(self):
        """toPythonStringLiteral method is used for creating string literals
        that can be used in constructing executable Python code. Test here
        that the method works well for special characters (quotes and backslashes)
        by using the generated string literal to set a variable.
        """

        test_strings = [
            "test with a ' single quote",
            'test with a " double quote',
            'test with both single \' and double " quotes',
            'test backslash \\ and \'single\' and "double" quotes'
            "'test string in single quotes'"
            '"test string in double quotes"' ]

        for test_string in test_strings:
            test_string_literal = slicer.app.pythonManager().toPythonStringLiteral(test_string)
            exec("test_string_literal_value = "+test_string_literal, globals())
            print("Test: "+test_string+" -> "+test_string_literal+" -> "+test_string_literal_value)
            self.assertEqual(test_string, test_string_literal_value)

    def tearDown(self):
        pass
