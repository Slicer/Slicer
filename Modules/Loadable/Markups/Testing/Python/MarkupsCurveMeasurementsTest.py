from __future__ import print_function
from slicer.util import TESTING_DATA_URL
import os

#
# Test curvature computation for curve markups
#

# Download test scene
curveMeasurementsTestDir = slicer.app.temporaryPath + '/curveMeasurementsTest'
print('Test directory: ', curveMeasurementsTestDir)
if not os.access(curveMeasurementsTestDir, os.F_OK):
  os.mkdir(curveMeasurementsTestDir)

testSceneFilePath = curveMeasurementsTestDir + '/MarkupsCurvatureTestScene.mrb'

slicer.util.downloadFile(
  TESTING_DATA_URL + 'SHA256/5b1f39e28ad8611790152fdc092ec9b3ee14254aad4897377db9576139c88e32',
  testSceneFilePath,
  checksum='SHA256:5b1f39e28ad8611790152fdc092ec9b3ee14254aad4897377db9576139c88e32')

# Import test scene
slicer.util.loadScene(testSceneFilePath)
curveNode = slicer.util.getNode('C')

# Check number of arrays in the curve node
curvePolyData = curveNode.GetCurveWorld()
curvePointData = curvePolyData.GetPointData()
if curvePointData.GetNumberOfArrays() != 1:
  exceptionMessage = "Unexpected number of data arrays in curve: " + str(curvePointData.GetNumberOfArrays())
  raise Exception(exceptionMessage)

# Turn on curvature calculation in curve node
curveNode.SetCalculateCurvature(True)

# Check curvature computation result
if curvePointData.GetNumberOfArrays() != 2:
  exceptionMessage = "Unexpected number of data arrays in curve: " + str(curvePointData.GetNumberOfArrays())
  raise Exception(exceptionMessage)

if curvePointData.GetArrayName(1) != 'Curvature':
  exceptionMessage = "Unexpected data array name in curve: " + str(curvePointData.GetArrayName(1))
  raise Exception(exceptionMessage)

curvatureArray = curvePointData.GetArray(1)
if curvatureArray.GetMaxId() != curvePointData.GetNumberOfTuples()-1:
  exceptionMessage = "Unexpected number of values in curvature data array: " + str(curvatureArray.GetMaxId())
  raise Exception(exceptionMessage)

if abs(curvatureArray.GetMaxNorm() - 0.9816015970208652) > 0.0001:
  exceptionMessage = "Unexpected maximum norm in curvature data array: " + str(curvatureArray.GetMaxNorm())
  raise Exception(exceptionMessage)

# Turn off curvature computation
curveNode.SetCalculateCurvature(False)
if curvePointData.GetNumberOfArrays() != 1:
  exceptionMessage = "Unexpected number of data arrays in curve: " + str(curvePointData.GetNumberOfArrays())
  raise Exception(exceptionMessage)
