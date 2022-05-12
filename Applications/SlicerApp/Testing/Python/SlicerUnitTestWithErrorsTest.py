import unittest


class SlicerUnitTestWithErrorsTest(unittest.TestCase):
  """ See https://docs.python.org/library/unittest.html#basic-example
  """

  def test_expectedtofail(self):
    self.assertTrue(False)
