import unittest
import random

class SlicerUnitTestTest(unittest.TestCase):
  """ See http://docs.python.org/library/unittest.html#basic-example
  """
  def setUp(self):
    self.seq = list(range(10))

  def test_shuffle(self):
    # Make sure the shuffled sequence does not lose any elements
    random.shuffle(self.seq)
    self.seq.sort()
    self.assertEqual(self.seq, list(range(10)))

    # Should raise an exception for an immutable sequence
    self.assertRaises(TypeError, random.shuffle, (1,2,3))

  def test_choice(self):
    element = random.choice(self.seq)
    self.assertIn(element, self.seq)

  def test_sample(self):
    self.assertRaises(ValueError, random.sample, self.seq, 20)
    for element in random.sample(self.seq, 5):
      self.assertIn(element, self.seq)

