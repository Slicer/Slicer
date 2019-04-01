from __future__ import print_function
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *


class VolumesLogicCompareVolumeGeometryTesting(ScriptedLoadableModuleTest):

  def setUp(self):
    pass

  def test_VolumesLogicCompareVolumeGeometry(self):
    """
    Load a volume, then call the compare volume geometry test with
    different values of epsilon and precision.
    """
    self.delayDisplay("Starting the test")

    #
    # first, get some sample data
    #
    import SampleData
    head = SampleData.downloadSample("MRHead")

    #
    # get the volumes logic and print out default epsilon and precision
    #
    volumesLogic = slicer.modules.volumes.logic()
    print('Compare volume geometry epsilon: ', volumesLogic.GetCompareVolumeGeometryEpsilon())
    print('Compare volume geometry precision: ', volumesLogic.GetCompareVolumeGeometryPrecision())
    self.assertAlmostEqual(volumesLogic.GetCompareVolumeGeometryEpsilon(), 1e-6)
    self.assertEqual(volumesLogic.GetCompareVolumeGeometryPrecision(), 6)

    #
    # compare the head against itself, this shouldn't produce any warning
    # string
    #
    warningString = volumesLogic.CompareVolumeGeometry(head, head)
    if len(warningString) != 0:
      print('Error in checking MRHead geometry against itself')
      print(warningString)
      return False
    else:
      print('Success in comparing MRHead vs itself with epsilon',volumesLogic.GetCompareVolumeGeometryEpsilon())

    #
    # see if you can get it to fail with a tighter epsilon
    #
    volumesLogic.SetCompareVolumeGeometryEpsilon(1e-10)
    precision = volumesLogic.GetCompareVolumeGeometryPrecision()
    if precision != 10:
      print('Error in calculating precision from epsilon of ', volumesLogic.GetCompareVolumeGeometryEpsilon(), ', expected 10, got ', precision)
      return False
    warningString = volumesLogic.CompareVolumeGeometry(head, head)
    if len(warningString) != 0:
      print('Error in checking MRHead geometry against itself with strict epsilon')
      print(warningString)
      return False
    else:
      print('Success in comparing MRHead vs itself with epsilon',volumesLogic.GetCompareVolumeGeometryEpsilon())



    #
    # clone the volume so can test for mismatches in geometry with
    # that operation
    #
    head2 = volumesLogic.CloneVolume(head, 'head2')

    warningString  = volumesLogic.CompareVolumeGeometry(head, head2)
    if len(warningString) != 0:
      print('Error in checking MRHead geometry against itself with epsilon ', volumesLogic.GetCompareVolumeGeometryEpsilon())
      print(warningString)
      return False
    else:
      print('Success in comparing MRHead vs clone with epsilon',volumesLogic.GetCompareVolumeGeometryEpsilon())

    #
    # now try with a label map volume
    #
    headLabel = volumesLogic.CreateAndAddLabelVolume(head, "label vol")
    warningString = volumesLogic.CompareVolumeGeometry(head, headLabel)
    if len(warningString) != 0:
      print('Error in comparing MRHead geometry against a label map of itself with epsilon',volumesLogic.GetCompareVolumeGeometryEpsilon())
      print(warningString)
      return False
    else:
      print('Success in comparing MRHead vs label map with epsilon',volumesLogic.GetCompareVolumeGeometryEpsilon())

    #
    # adjust the geometry and make it fail
    #
    head2Matrix = vtk.vtkMatrix4x4()
    head2.GetRASToIJKMatrix(head2Matrix)
    val = head2Matrix.GetElement(2,0)
    head2Matrix.SetElement(2,0,val+0.25)
    head2.SetRASToIJKMatrix(head2Matrix)
    head2.SetSpacing(0.12345678901234567890, 2.0, 3.4)

    warningString = volumesLogic.CompareVolumeGeometry(head,head2)
    if len(warningString) == 0:
      print('Error in comparing MRHead geometry against an updated clone, with epsilon',volumesLogic.GetCompareVolumeGeometryEpsilon())
      return False
    else:
      print('Success in making the comparison fail, with with epsilon',volumesLogic.GetCompareVolumeGeometryEpsilon())
      print(warningString)

    #
    # reset the epsilon with an invalid negative number
    #
    volumesLogic.SetCompareVolumeGeometryEpsilon(-0.01)
    epsilon = volumesLogic.GetCompareVolumeGeometryEpsilon()
    if epsilon != 0.01:
      print('Failed to use the absolute value for an epsilon of -0.01: ', epsilon)
      return False
    precision = volumesLogic.GetCompareVolumeGeometryPrecision()
    if precision != 2:
      print('Failed to set the precision to 2: ',precision)
      return False
    warningString = volumesLogic.CompareVolumeGeometry(head,head2)
    print(warningString)

    self.delayDisplay('Test passed')

    return True
