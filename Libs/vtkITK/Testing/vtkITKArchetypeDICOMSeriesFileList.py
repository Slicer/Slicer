"""Test reading a DICOM series from a list of files that is not in the order of slice positions."""

import glob
import os
import re
import unittest

import vtkITK


class vtkITKArchetypeDICOMSeriesFileList(unittest.TestCase):
    def setUp(self):
        dataDir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Testing", "Data", "Input", "CTHeadAxialDicom")
        # CTHead<N>.dcm is the N-th slice (slice position is (N - 1) * 1.5 mm).
        # Sorting the file names alphabetically puts the slices out of order (CTHead1, CTHead10, CTHead11, ...).
        self.fileNames = sorted(glob.glob(os.path.join(dataDir, "CTHead*.dcm")))
        self.assertEqual(len(self.fileNames), 93)

    def sliceNumber(self, fileName):
        return int(re.search(r"CTHead(\d+)\.dcm$", fileName).group(1))

    def test_file_list_sorted_by_position(self):
        reader = vtkITK.vtkITKArchetypeImageSeriesScalarReader()
        reader.SetArchetype(self.fileNames[0])
        for fileName in self.fileNames:
            reader.AddFileName(fileName)
        reader.SetSingleFile(0)
        reader.SetOutputScalarTypeToNative()
        reader.SetDesiredCoordinateOrientationToNative()
        reader.SetUseNativeOriginOn()
        reader.Update()

        self.assertEqual(reader.GetOutput().GetDimensions()[2], 93)
        # The files are used in the order of slice positions, not in the order they were specified
        sliceNumbers = [self.sliceNumber(reader.GetFileName(i)) for i in range(reader.GetNumberOfFileNames())]
        self.assertEqual(sliceNumbers, list(range(1, 94)))

    def runTest(self):
        self.setUp()
        self.test_file_list_sorted_by_position()
