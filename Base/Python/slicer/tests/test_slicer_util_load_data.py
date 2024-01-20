import tempfile
import unittest
from pathlib import Path
from urllib.parse import urljoin

import slicer


class SlicerUtilLoadDataTests(unittest.TestCase):

    def setUp(self):
        pass

    def test_loadFiberBundle(self):

        def _load(fiberBundleFilePath):

            node = slicer.util.loadFiberBundle(fiberBundleFilePath)
            self.assertIsInstance(node, slicer.vtkMRMLFiberBundleNode)
            node = slicer.util.loadFiberBundle(fiberBundleFilePath)
            self.assertIsInstance(node, slicer.vtkMRMLFiberBundleNode)

        hash_algorithm = "SHA256"
        checksum = "e1848e4d0b90d6c7442b2474c513d0464c6f141253c25aee6f5bcda4c828deac"
        url = urljoin(slicer.util.TESTING_DATA_URL, f"{hash_algorithm}/{checksum}")

        with tempfile.TemporaryDirectory() as tmpdirname:

            file_basename = "tractography.vtk"
            fiberBundleFilePath = Path(tmpdirname).joinpath(file_basename)
            downloadSuccess = slicer.util.downloadFile(
                url, fiberBundleFilePath, checksum=f"{hash_algorithm}:{checksum}", reDownloadIfChecksumInvalid=True)
            self.assertTrue(downloadSuccess)

            if slicer.app.ioManager().registeredFileReaderCount("FiberBundleFile") == 0:
                with self.assertRaisesRegex(RuntimeError, "FiberBundleFile reader not registered") as cm:
                    _load(fiberBundleFilePath)
            else:
                _load(fiberBundleFilePath)
