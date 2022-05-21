from slicer.util import TESTING_DATA_URL
import os
import numpy as np

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
curvePointData = curveNode.GetCurveWorld().GetPointData()
if curvePointData.GetNumberOfArrays() != 1:
    exceptionMessage = f"Unexpected number of data arrays in curve: {curvePointData.GetNumberOfArrays()} (expected 1)"
    raise Exception(exceptionMessage)

# Turn on curvature calculation in curve node
curveNode.GetMeasurement("curvature max").SetEnabled(True)

# Check curvature computation result
curvePointData = curveNode.GetCurveWorld().GetPointData()
if curvePointData.GetNumberOfArrays() != 2:
    exceptionMessage = f"Unexpected number of data arrays in curve: {curvePointData.GetNumberOfArrays()} (expected 2)"
    raise Exception(exceptionMessage)

if curvePointData.GetArrayName(1) != 'Curvature':
    exceptionMessage = f"Unexpected data array name in curve: {curvePointData.GetArrayName(1)} (expected 'Curvature')"
    raise Exception(exceptionMessage)

curvatureArray = curvePointData.GetArray(1)
if curvatureArray.GetMaxId() != curvePointData.GetNumberOfTuples() - 1:
    exceptionMessage = "Unexpected number of values in curvature data array: %d (expected %d)" % (curvatureArray.GetMaxId(), curvePointData.GetNumberOfTuples() - 1)
    raise Exception(exceptionMessage)

if abs(curvatureArray.GetRange()[0] - 0.0) > 0.0001:
    exceptionMessage = "Unexpected minimum in curvature data array: " + str(curvatureArray.GetRange()[0])
    raise Exception(exceptionMessage)
if abs(curvatureArray.GetRange()[1] - 0.9816015970208652) > 0.0001:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(curvatureArray.GetRange()[1])
    raise Exception(exceptionMessage)

# Turn off curvature computation
curveNode.GetMeasurement("curvature max").SetEnabled(False)
curvePointData = curveNode.GetCurveWorld().GetPointData()
if curvePointData.GetNumberOfArrays() != 1:
    exceptionMessage = "Unexpected number of data arrays in curve: " + str(curvePointData.GetNumberOfArrays())
    raise Exception(exceptionMessage)

print('Open curve curvature test finished successfully')

#
# Test closed curve curvature computation
#

closedCurveNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsClosedCurveNode')
pos = np.zeros(3)
for i in range(curveNode.GetNumberOfControlPoints()):
    curveNode.GetNthControlPointPosition(i, pos)
    closedCurveNode.AddControlPoint(vtk.vtkVector3d(pos))

closedCurveNode.GetMeasurement("curvature mean").SetEnabled(True)
curvePointData = closedCurveNode.GetCurveWorld().GetPointData()
if curvePointData.GetNumberOfArrays() != 2:
    exceptionMessage = "Unexpected number of data arrays in curve: " + str(curvePointData.GetNumberOfArrays())
    raise Exception(exceptionMessage)

if curvePointData.GetArrayName(1) != 'Curvature':
    exceptionMessage = "Unexpected data array name in curve: " + str(curvePointData.GetArrayName(1))
    raise Exception(exceptionMessage)

curvatureArray = curvePointData.GetArray(1)
if curvatureArray.GetMaxId() != curvePointData.GetNumberOfTuples() - 1:
    exceptionMessage = "Unexpected number of values in curvature data array: %d (expected %d)" % (curvatureArray.GetMaxId(), curvePointData.GetNumberOfTuples() - 1)
    raise Exception(exceptionMessage)

if abs(curvatureArray.GetRange()[0] - 0.0) > 0.0001:
    exceptionMessage = "Unexpected minimum in curvature data array: " + str(curvatureArray.GetRange()[0])
    raise Exception(exceptionMessage)
if abs(curvatureArray.GetRange()[1] - 0.26402460470400924) > 0.0001:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(curvatureArray.GetRange()[1])
    raise Exception(exceptionMessage)

print('Closed curve curvature test finished successfully')

#
# Test interpolation of control point measurements
#
# This test case uses a stripped scene that is the result of calculating radius
# of a model using the ExtractCenterline module in the SlicerVMTK extension.
#

slicer.mrmlScene.Clear()

testSceneFilePath = curveMeasurementsTestDir + '/MarkupsControlPointMeasurementInterpolationTestScene.mrb'

slicer.util.downloadFile(
    TESTING_DATA_URL + 'SHA256/b636ecfc1be54504c2c9843e1ff53242ee6b951228490ae99a89e06c8890e344',
    testSceneFilePath,
    checksum='SHA256:b636ecfc1be54504c2c9843e1ff53242ee6b951228490ae99a89e06c8890e344')

# Import test scene
slicer.util.loadScene(testSceneFilePath)

centerlineModel = slicer.util.getNode('Centerline model')
centerlinePolyData = centerlineModel.GetPolyData()

# Create curve node from centerline model
# (markups curve with radius array output is not yet available in SlicerVMTK)
centerlineCurve = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsCurveNode')
centerlineCurve.SetName('CenterlineCurve')
for i in range(centerlinePolyData.GetNumberOfPoints()):
    centerlineCurve.AddControlPoint(vtk.vtkVector3d(centerlinePolyData.GetPoint(i)))

# Add radius data to centerline curve as measurement
radiusMeasurement = slicer.vtkMRMLStaticMeasurement()
radiusMeasurement.SetName('Radius')
radiusMeasurement.SetUnits('mm')
radiusMeasurement.SetPrintFormat('')  # Prevent from showing up in SH Description
radiusMeasurement.SetControlPointValues(centerlinePolyData.GetPointData().GetArray('Radius'))
centerlineCurve.AddMeasurement(radiusMeasurement)

centerlineCurvePointData = centerlineCurve.GetCurveWorld().GetPointData()

# Check interpolation computation result
if centerlineCurvePointData.GetNumberOfArrays() != 2:
    exceptionMessage = "Unexpected number of data arrays in curve: " + str(centerlineCurvePointData.GetNumberOfArrays())
    raise Exception(exceptionMessage)

if centerlineCurvePointData.GetArrayName(1) != 'Radius':
    exceptionMessage = "Unexpected data array name in curve: " + str(centerlineCurvePointData.GetArrayName(1))
    raise Exception(exceptionMessage)

interpolatedRadiusArray = centerlineCurvePointData.GetArray(1)
if interpolatedRadiusArray.GetNumberOfTuples() != 571:
    exceptionMessage = "Unexpected number of data points in interpolated radius array: " + str(interpolatedRadiusArray.GetNumberOfTuples())
    raise Exception(exceptionMessage)

if abs(interpolatedRadiusArray.GetRange()[0] - 12.322814731747465) > 0.0001:
    exceptionMessage = "Unexpected minimum in curvature data array: " + str(interpolatedRadiusArray.GetRange()[0])
    raise Exception(exceptionMessage)
if abs(interpolatedRadiusArray.GetRange()[1] - 42.9542138185081) > 0.0001:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(interpolatedRadiusArray.GetRange()[1])
    raise Exception(exceptionMessage)
if abs(interpolatedRadiusArray.GetValue(9) - 42.92838813390291) > 0.0001:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(interpolatedRadiusArray.GetValue(9))
    raise Exception(exceptionMessage)
if abs(interpolatedRadiusArray.GetValue(10) - 42.9542138185081) > 0.0001:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(interpolatedRadiusArray.GetValue(10))
    raise Exception(exceptionMessage)
if abs(interpolatedRadiusArray.GetValue(569) - 12.904227531040913) > 0.0001:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(interpolatedRadiusArray.GetValue(569))
    raise Exception(exceptionMessage)
if abs(interpolatedRadiusArray.GetValue(570) - 12.765926543271583) > 0.0001:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(interpolatedRadiusArray.GetValue(570))
    raise Exception(exceptionMessage)

print('Control point measurement interpolation test finished successfully')

#
# Test curvature computation for a circle-shaped closed curve
#

radius = 35
numberOfControlPoints = 40
import math
closedCurveNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsClosedCurveNode")
for controlPointIndex in range(numberOfControlPoints):
    angle = 2.0 * math.pi * controlPointIndex / numberOfControlPoints
    closedCurveNode.AddControlPoint(vtk.vtkVector3d(radius * math.sin(angle), radius * math.cos(angle), 0.0))

# Turn on curvature calculation in curve node
closedCurveNode.GetMeasurement("curvature mean").SetEnabled(True)
closedCurveNode.GetMeasurement("curvature max").SetEnabled(True)
curvatureArray = closedCurveNode.GetCurveWorld().GetPointData().GetArray('Curvature')
if curvatureArray.GetNumberOfValues() < 10:
    exceptionMessage = "Many values are expected in the curvature array, instead found just %d" % curvatureArray.GetNumberOfValues()
    raise Exception(exceptionMessage)

if abs(curvatureArray.GetRange()[0] - 1 / radius) > 1e-4:
    exceptionMessage = "Unexpected minimum in curvature data array: " + str(curvatureArray.GetRange()[0])
    raise Exception(exceptionMessage)
if abs(curvatureArray.GetRange()[1] - 1 / radius) > 1e-4:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(curvatureArray.GetRange()[1])
    raise Exception(exceptionMessage)
if abs(curvatureArray.GetRange()[1] - 1 / radius) > 1e-4:
    exceptionMessage = "Unexpected maximum in curvature data array: " + str(curvatureArray.GetRange()[1])
    raise Exception(exceptionMessage)
if abs(closedCurveNode.GetMeasurement("curvature mean").GetValue() - 1 / radius) > 1e-4:
    exceptionMessage = "Unexpected curvature mean value: " + str(closedCurveNode.GetMeasurement("curvature mean").GetValue())
    raise Exception(exceptionMessage)
if abs(closedCurveNode.GetMeasurement("curvature max").GetValue() - 1 / radius) > 1e-4:
    exceptionMessage = "Unexpected curvature max value: " + str(closedCurveNode.GetMeasurement("curvature max").GetValue())
    raise Exception(exceptionMessage)

# Check length and area

closedCurveNode.GetMeasurement("length").SetEnabled(True)
if closedCurveNode.GetMeasurement("length").GetValueWithUnitsAsPrintableString() != '219.9mm':
    exceptionMessage = "Unexpected curve length value: " + closedCurveNode.GetMeasurement("length").GetValueWithUnitsAsPrintableString()
    raise Exception(exceptionMessage)

closedCurveNode.GetMeasurement("area").SetEnabled(True)
if closedCurveNode.GetMeasurement("area").GetValueWithUnitsAsPrintableString() != '38.48cm2':
    exceptionMessage = "Unexpected curve area value: " + closedCurveNode.GetMeasurement("area").GetValueWithUnitsAsPrintableString()
    raise Exception(exceptionMessage)

# Display surface area as a model.
# Useful for manual testing of surface quality.
closedCurveNode.GetDisplayNode().SetSnapMode(slicer.vtkMRMLMarkupsDisplayNode.SnapModeUnconstrained)
areaMeasurement = closedCurveNode.GetMeasurement("area")
areaMeasurement.SetEnabled(True)
areaModelNode = slicer.modules.models.logic().AddModel(areaMeasurement.GetMeshValue())
areaModelNode.GetDisplayNode().SetEdgeVisibility(True)

print('Radius of curvature computation is verified successfully')
