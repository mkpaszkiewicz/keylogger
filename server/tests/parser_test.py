#!/usr/bin/env python
import unittest
import os
import sys

sys.path.insert(0, os.path.pardir)

import server


class ParserTest(unittest.TestCase):
    """
    Class tests server.parse function. Checks whether bytes stream is properly transformed to readable format.
    """

    def test_parser1(self):
        data = b'\x01\x1E\x01\x1F\x01\x20\x01\x21\x01\x22\x01\x23\x01\x24\x01\x25\x01\x26\x01\x27'
        result = server.parse(data)
        self.assertEqual(result, 'asdfghjkl;')

    def test_parser2(self):
        data = b'\x01\x1E\x00\x1E\x01\x1F\x00\x1F\x01\x20\x00\x20\x01\x21\x00\x21\x01\x22\x00\x22'
        result = server.parse(data)
        self.assertEqual(result, 'asdfg')

    def test_parser3(self):
        data = b'\x01\x1E\x02\x1E\x02\x1E\x02\x1E\x02\x1E\x02\x1E\x02\x1E\x02\x1E\x02\x1E\x02\x1E'
        result = server.parse(data)
        self.assertEqual(result, 'aaaaaaaaaa')

    def test_parser4(self):
        data = b'\x01\x1E\x02\x1E\x00\x1E\x01\x1F\x02\x1F\x00\x1F\x01\x20\x02\x20\x00\x20\x01\x21\x02\x21\x00\x21'
        result = server.parse(data)
        self.assertEqual(result, 'aassddff')

    def test_parser5(self):
        data = b'\x01\x1E\x02\x1E\x00\x1E\x03\xFF\x01\x1F\x02\x1F'
        result = server.parse(data)
        self.assertEqual(result, 'aa OMITTED: 255 ss')

    def test_parser6(self):
        data = b'\x01\x1E\x02\x1E\x00\x1E\x03\xFF\x01\x1F\x02\x1F\x03\x10'
        result = server.parse(data)
        self.assertEqual(result, 'aa OMITTED: 255 ss OMITTED: 16 ')

    def test_parser7(self):
        data = b'\x01\x1E\x02\x1E\x00\x1E\x03\xFF\x01\x1F\x02\x1F\x03\x10'
        result = server.parse(data)
        self.assertEqual(result, 'aa OMITTED: 255 ss OMITTED: 16 ')

    def test_parser8(self):
        data = b''
        result = server.parse(data)
        self.assertEqual(result, '')

    def test_parser9(self):
        data = b'\x04\x1E'
        self.assertRaises(KeyError, server.parse, data)

    def test_parser10(self):
        data = b'\x01\x59'
        self.assertRaises(KeyError, server.parse, data)

    def test_parser11(self):
        data = b'\x01\x1E\x01'
        self.assertRaises(IndexError, server.parse, data)


if __name__ == '__main__':
    unittest.main()