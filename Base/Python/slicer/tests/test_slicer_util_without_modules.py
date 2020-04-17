import io
import os
import tempfile
import unittest
import slicer.util


class SlicerUtilWithoutModulesTest(unittest.TestCase):

  def setUp(self):
    pass

  def test_computeChecksum(self):
    with self.assertRaises(IOError):
      slicer.util.computeChecksum('SHA256', 'compute-checksum-nonexistent.txt')

    with tempfile.TemporaryDirectory() as tmpdirname:

      input_file = os.path.join(tmpdirname, 'compute-checksum.txt')
      with io.open(input_file, 'w', newline='\n') as content:
        content.write('This is a text file!\n')

      self.assertTrue(os.path.exists(input_file))

      self.assertEqual(slicer.util.computeChecksum('SHA256', input_file), '4a57f3207b97f26a6061f86948483c00b03893ddfef9e82b639ebe66e3aba338')
      self.assertEqual(slicer.util.computeChecksum('SHA512', input_file), '5080ee92e951c5f8336053f11d278c23f5d26b5eb78805c952960eac0194f357f98b0e350611ce081d4a1e28dd8ea182d3a276c99b1752e0def2de0f47b8b27b')

      with self.assertRaises(ValueError):
        slicer.util.computeChecksum('SHAINVALID', input_file)

  def test_extractAlgoAndDigest(self):
    with self.assertRaises(ValueError):
      slicer.util.extractAlgoAndDigest('4a57f3207b97f26a6061f86948483c00b03893ddfef9e82b639ebe66e3aba338')

    with self.assertRaises(ValueError):
      slicer.util.extractAlgoAndDigest('SHAINVALID:4a57f3207b97f26a6061f86948483c00b03893ddfef9e82b639ebe66e3aba338')

    self.assertEqual(
      slicer.util.extractAlgoAndDigest('SHA256:4a57f3207b97f26a6061f86948483c00b03893ddfef9e82b639ebe66e3aba338'),
      ('SHA256', '4a57f3207b97f26a6061f86948483c00b03893ddfef9e82b639ebe66e3aba338'))
