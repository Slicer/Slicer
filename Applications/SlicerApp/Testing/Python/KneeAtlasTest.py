from __future__ import print_function
import unittest
import slicer
import AtlasTests

class testClass(object):
  """ Run the knee atlas test by itself
  """

  def setUp(self):
    print("Import the atlas tests...")
    atlasTests = AtlasTests.AtlasTestsTest()
    atlasTests.test_KneeAtlasTest()

class KneeAtlasTest(unittest.TestCase):

  def setUp(self):
    pass

  def test_testClass(self):
    test = testClass()
    test.setUp()
