import unittest


class SlicerPythonLzmaTests(unittest.TestCase):
  """This test verifies that Python is build with lzma enabled.
  """

  def setUp(self):
    pass

  def tearDown(self):
    pass

  def test_compressionDecompressionRoundtrip(self):
    # Generate some input data
    someText = "something..."
    originalData = someText.encode()

    # Compress
    import lzma
    lzc = lzma.LZMACompressor()
    compressedData = lzc.compress(originalData) + lzc.flush()

    # Uncompress
    lzd = lzma.LZMADecompressor()
    uncompressedData = lzd.decompress(compressedData)

    # Test if data after compression&decompression is the same as the original
    self.assertEqual(originalData, uncompressedData)
