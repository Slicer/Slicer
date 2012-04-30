import unittest

class SlicerUnitTestWithErrorsTest(unittest.TestCase):
  """ See http://docs.python.org/library/unittest.html#basic-example
  """

  def test_expectedtofail(self):
    self.assertTrue(False)

