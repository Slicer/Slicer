#Testing against the NRRD reader
import unittest
import slicer
import vtkITK
import vtkTeem
from vtk.util import numpy_support as ns
import numpy

"""
To run as test from slicer python console, replace the following with your source tree path and paste:

exec(open('/Users/pieper/slicer4/latest/Slicer/Libs/vtkITK/Testing/vtkITKArchetypeScalarReaderFile.py').read()); t = vtkITKReaderAgainstNRRDReader(); t.runTest()

note that from the 't' variable in the console you can access the readers and other instance variables for debugging.
"""

class vtkITKReaderAgainstNRRDReader(unittest.TestCase):
    def setUp(self):
        from SampleData import SampleDataLogic
        brainSource = SampleDataLogic().sourceForSampleName('MRHead')
        self.file_name = SampleDataLogic().downloadSourceIntoCache(brainSource)[0]

        self.ritk = vtkITK.vtkITKArchetypeImageSeriesScalarReader()
        self.ritk.SetUseOrientationFromFile(True)
        self.ritk.SetUseNativeOriginOn();
        self.ritk.SetOutputScalarTypeToNative()
        self.ritk.SetDesiredCoordinateOrientationToNative()
        self.ritk.SetArchetype(self.file_name)
        self.ritk.Update()

        self.rnrrd = vtkTeem.vtkTeemNRRDReader()
        self.rnrrd.SetFileName(self.file_name)
        self.rnrrd.Update()

        self.assertTrue(
            compare_vtk_matrix(
                self.ritk.GetRasToIjkMatrix(),
                self.rnrrd.GetRasToIjkMatrix()
            )
        )

    def test_pointdata(self):
        self.nrrdArray = ns.vtk_to_numpy(self.rnrrd.GetOutput().GetPointData().GetScalars())
        self.itkArray = ns.vtk_to_numpy(self.ritk.GetOutput().GetPointData().GetScalars())
        self.assertTrue(numpy.allclose(self.nrrdArray, self.itkArray))

    def runTest(self):
      self.setUp()
      self.test_pointdata()
      self.test_ras_to_ijk()



def compare_vtk_matrix(m1, m2, n=4):
    for i in range(0,n):
        for j in range(0,n):
            assert m1.GetElement(i,j) == m2.GetElement(i,j)
    return True




