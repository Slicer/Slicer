import unittest
import slicer
import AtlasTests

class testClass():
  """ Run the abdominal atlas test by itself
  """

  def setUp(self):
    print("Import the atlas tests...")
    atlasTests = AtlasTests.AtlasTestsTest()
    atlasTests.test_AbdominalAtlasTest()

class AbdominalAtlasTest(unittest.TestCase):

  def setUp(self):
    pass

  def test_testClass(self):
    test = testClass()
    test.setUp()
