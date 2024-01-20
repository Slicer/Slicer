import tempfile
import unittest
from pathlib import Path
from urllib.parse import urljoin

import slicer


class SlicerUtilLoadDataTests(unittest.TestCase):

    def setUp(self):
        pass

    def test_loadFiberBundle(self):

        def _checkNode(node, returnNode):

            if returnNode:
                # Deprecated
                self.assertIsInstance(node, tuple)
                self.assertEqual(len(node), 2)
                self.assertTrue(node[0])
                node = node[1]
            self.assertIsInstance(node, slicer.vtkMRMLFiberBundleNode)

        def _load(fiberBundleFilePath, returnNode=False):

            node = slicer.util.loadFiberBundle(fiberBundleFilePath, returnNode=returnNode)
            _checkNode(node, returnNode)

            node = slicer.util.loadFiberBundle(fiberBundleFilePath, returnNode=returnNode)
            _checkNode(node, returnNode)

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

                with self.assertRaisesRegex(RuntimeError, "FiberBundleFile reader not registered") as cm:
                    # Deprecated
                    _load(fiberBundleFilePath, returnNode=True)

            else:
                _load(fiberBundleFilePath)

                # Deprecated
                _load(fiberBundleFilePath, returnNode=True)
