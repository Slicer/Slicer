#Testing against the NRRD reader
import unittest
import slicer
import vtkITK
from vtk.util import numpy_support as ns
import numpy

class vtkITKReaderAgainstNRRDReader(unittest.TestCase):
    def setUp(self):
        import SampleData
        sampleDataLogic = SampleData.SampleDataLogic()
        dti = sampleDataLogic.downloadMRHead()
        self.file_name = dti.GetStorageNode().GetFileName()

        self.ritk = vtkITK.vtkITKArchetypeImageSeriesScalarReader()
        self.ritk.SetArchetype(self.file_name)
        self.ritk.Update()

        self.rnrrd = slicer.vtkNRRDReader()
        self.rnrrd.SetFileName(self.file_name)
        self.rnrrd.Update()

    def test_measurement_frame(self):
        self.assertTrue(
            compare_vtk_matrix(
                self.ritk.GetMeasurementFrameMatrix(),
                self.rnrrd.GetMeasurementFrameMatrix()
            )
        )

    def test_ras_to_ijk(self):
        print "ITK Matrix"
        print self.ritk.GetRasToIjkMatrix(),
        print "NRRD Reader Matrix"
        print self.rnrrd.GetRasToIjkMatrix()

        self.assertTrue(
            compare_vtk_matrix(
                self.ritk.GetRasToIjkMatrix(),
                self.rnrrd.GetRasToIjkMatrix()
            )
        )

    def test_pointdata(self):
        self.assertTrue(numpy.allclose(
            ns.vtk_to_numpy(self.rnrrd.GetOutput().GetPointData().GetScalars()),
            ns.vtk_to_numpy(self.ritk.GetOutput().GetPointData().GetScalars())
        ))




def compare_vtk_matrix(m1, m2, n=4):
    for i in xrange(0,n):
        for j in xrange(0,n):
            assert(m1.GetElement(i,j) == m2.GetElement(i,j))




