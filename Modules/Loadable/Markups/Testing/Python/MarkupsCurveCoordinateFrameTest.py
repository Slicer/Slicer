from slicer.util import TESTING_DATA_URL
import os
import numpy as np

#
# Test computation of coordinate system (normal, binormal, and tangent vector) along the curve.
# The test can be used to visually check correctness of curve coordinate frame computation.
# Coordinate frame axes are updated in real-time as the control points of curves are moved.
#

# Helper functions to Visualize coordinate system frame along a curve


def updateCoordinateSystemsModel(updateInfo):
    model, curve, coordinateSystemAppender, curvePointToWorldTransform, transform, transformer = updateInfo
    coordinateSystemAppender.RemoveAllInputs()
    numberOfCurvePoints = curve.GetCurvePointsWorld().GetNumberOfPoints()
    for curvePointIndex in range(numberOfCurvePoints):
        result = curve.GetCurvePointToWorldTransformAtPointIndex(curvePointIndex, curvePointToWorldTransform)
        transform.SetMatrix(curvePointToWorldTransform)
        transformer.Update()
        coordinateSystemInWorld = vtk.vtkPolyData()
        coordinateSystemInWorld.DeepCopy(transformer.GetOutput())
        coordinateSystemAppender.AddInputData(coordinateSystemInWorld)
    coordinateSystemAppender.Update()
    model.SetAndObservePolyData(coordinateSystemAppender.GetOutput())


def createCoordinateSystemsModel(curve, axisLength=5):
    """Add a coordinate system model at each curve point.
    :param curve: input curve
    :param axisLength: length of normal axis is `axisLength*2`, length of binormal and tangent axes are `axisLength`
    :return model, coordinateSystemAppender, curvePointToWorldTransform, transform
    """
    # Create coordinate system polydata
    axisAppender = vtk.vtkAppendPolyData()
    xAxis = vtk.vtkLineSource()
    xAxis.SetPoint1(0, 0, 0)
    xAxis.SetPoint2(axisLength * 2, 0, 0)
    yAxis = vtk.vtkLineSource()
    yAxis.SetPoint1(0, 0, 0)
    yAxis.SetPoint2(0, axisLength, 0)
    zAxis = vtk.vtkLineSource()
    zAxis.SetPoint1(0, 0, 0)
    zAxis.SetPoint2(0, 0, axisLength)
    axisAppender.AddInputConnection(xAxis.GetOutputPort())
    axisAppender.AddInputConnection(yAxis.GetOutputPort())
    axisAppender.AddInputConnection(zAxis.GetOutputPort())
    # Initialize transformer that will place the coordinate system polydata along the curve
    curvePointToWorldTransform = vtk.vtkMatrix4x4()
    transformer = vtk.vtkTransformPolyDataFilter()
    transform = vtk.vtkTransform()
    transformer.SetTransform(transform)
    transformer.SetInputConnection(axisAppender.GetOutputPort())
    # Create model appender that assembles the model that contains all the coordinate systems
    coordinateSystemAppender = vtk.vtkAppendPolyData()
    # model = slicer.modules.models.logic().AddModel(coordinateSystemAppender.GetOutputPort())
    model = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLModelNode')
    model.CreateDefaultDisplayNodes()
    # prevent picking by markups so that the control points can be moved without sticking to the generated model
    model.SetSelectable(False)
    updateInfo = [model, curve, coordinateSystemAppender, curvePointToWorldTransform, transform, transformer]
    updateCoordinateSystemsModel(updateInfo)
    return updateInfo


def addCoordinateSystemUpdater(updateInfo):
    model, curve, coordinateSystemAppender, curvePointToWorldTransform, transform, transformer = updateInfo
    observation = curve.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent,
                                    lambda caller, eventData, updateInfo=updateInfo: updateCoordinateSystemsModel(updateInfo))
    return [curve, observation]


def removeCoordinateSystemUpdaters(curveObservations):
    for curve, observer in curveObservations:
        curve.RemoveObserver(observer)


#
# Set up tests
#

curveMeasurementsTestDir = slicer.app.temporaryPath + '/curveMeasurementsTest'
print('Test directory: ', curveMeasurementsTestDir)
if not os.access(curveMeasurementsTestDir, os.F_OK):
    os.mkdir(curveMeasurementsTestDir)

curvePointToWorldTransform = vtk.vtkMatrix4x4()

curveObservations = []

#
# Test1. Test free-form curve with 6 control points, all points in one plane
#

testSceneFilePath = curveMeasurementsTestDir + '/MarkupsCurvatureTestScene.mrb'
slicer.util.downloadFile(
    TESTING_DATA_URL + 'SHA256/5b1f39e28ad8611790152fdc092ec9b3ee14254aad4897377db9576139c88e32',
    testSceneFilePath,
    checksum='SHA256:5b1f39e28ad8611790152fdc092ec9b3ee14254aad4897377db9576139c88e32')
slicer.util.loadScene(testSceneFilePath)
planarCurveNode = slicer.util.getNode('C')

# Visualize

updateInfo = createCoordinateSystemsModel(planarCurveNode)
curveObservations.append(addCoordinateSystemUpdater(updateInfo))

# Check quantitative results

if not planarCurveNode.GetCurvePointToWorldTransformAtPointIndex(6, curvePointToWorldTransform):
    raise Exception("Test1 GetCurvePointToWorldTransformAtPointIndex failed")

curvePointToWorldMatrix = slicer.util.arrayFromVTKMatrix(curvePointToWorldTransform)
expectedCurvePointToWorldMatrix = np.array(
    [[2.15191499e-01, 0.00000000e+00, -9.76571871e-01, -3.03394470e+01],
     [0.00000000e+00, -1.00000000e+00, 0.00000000e+00, 3.63797881e-09],
        [-9.76571871e-01, 0.00000000e+00, -2.15191499e-01, 8.10291061e+01],
        [0.00000000e+00, 0.00000000e+00, 0.00000000e+00, 1.00000000e+00]])

if not np.isclose(curvePointToWorldMatrix, expectedCurvePointToWorldMatrix).all():
    raise Exception(f"Test1 CurvePointToWorldTransformAtPointIndex value incorrect: got {curvePointToWorldMatrix}, expected {expectedCurvePointToWorldMatrix}.")

#
# Test2. Test free-form closed curve with 6 control points, all points in one plane
#

closedCurveNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsClosedCurveNode')
pos = np.zeros(3)
for i in range(planarCurveNode.GetNumberOfControlPoints()):
    planarCurveNode.GetNthControlPointPosition(i, pos)
    pointIndex = closedCurveNode.AddControlPoint(vtk.vtkVector3d(pos))

# Visualize

updateInfo = createCoordinateSystemsModel(closedCurveNode)
curveObservations.append(addCoordinateSystemUpdater(updateInfo))

# Check quantitative results

if not closedCurveNode.GetCurvePointToWorldTransformAtPointIndex(6, curvePointToWorldTransform):
    raise Exception("Test2 GetCurvePointToWorldTransformAtPointIndex failed")

curvePointToWorldMatrix = slicer.util.arrayFromVTKMatrix(curvePointToWorldTransform)
expectedCurvePointToWorldMatrix = np.array(
    [[-3.85813409e-01, 0.00000000e+00, -9.22576833e-01, -3.71780586e+01],
     [0.00000000e+00, 1.00000000e+00, 0.00000000e+00, 3.63797881e-09],
        [9.22576833e-01, 0.00000000e+00, -3.85813409e-01, 8.78303909e+01],
        [0.00000000e+00, 0.00000000e+00, 0.00000000e+00, 1.00000000e+00]])

if not np.isclose(curvePointToWorldMatrix, expectedCurvePointToWorldMatrix).all():
    raise Exception(f"Test2 CurvePointToWorldTransformAtPointIndex value incorrect: got {curvePointToWorldMatrix}, expected {expectedCurvePointToWorldMatrix}.")

#
# Test3. Test on a vessel centerline curve
#
# This test case uses a stripped scene that is the result of calculating radius
# of a model using the ExtractCenterline module in the SlicerVMTK extension.
#

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
    pointIndex = centerlineCurve.AddControlPoint(vtk.vtkVector3d(centerlinePolyData.GetPoint(i)))

# Spacing between control points is not uniform. Applying b-spline interpolation to it directly would
# cause discontinuities (overshoots where a large gap between points is followed by points close to each other).
# To avoid this, we resample the points at equal distances (5mm) and then enable smooth bspline interpolation.
centerlineCurve.SetCurveTypeToLinear()
centerlineCurve.ResampleCurveWorld(5)
centerlineCurve.SetCurveTypeToCardinalSpline()

# Visualize

updateInfo = createCoordinateSystemsModel(centerlineCurve, 2)
curveObservations.append(addCoordinateSystemUpdater(updateInfo))

# Check quantitative results

if not centerlineCurve.GetCurvePointToWorldTransformAtPointIndex(6, curvePointToWorldTransform):
    raise Exception("Test3 GetCurvePointToWorldTransformAtPointIndex failed")

curvePointToWorldMatrix = slicer.util.arrayFromVTKMatrix(curvePointToWorldTransform)
expectedCurvePointToWorldMatrix = np.array(
    [[9.85648052e-01, 8.80625424e-03, -1.68583415e-01, -3.08991909e+00],
     [-6.35257659e-02, 9.44581803e-01, -3.22070946e-01, 2.22146526e-01],
        [1.56404587e-01, 3.28157991e-01, 9.31584638e-01, -7.34501495e+01],
        [0.00000000e+00, 0.00000000e+00, 0.00000000e+00, 1.00000000e+00]])

if not np.isclose(curvePointToWorldMatrix, expectedCurvePointToWorldMatrix).all():
    raise Exception(f"Test3 CurvePointToWorldTransformAtPointIndex value incorrect: got {curvePointToWorldMatrix}, expected {expectedCurvePointToWorldMatrix}.")

#
# Test4. curvature computation for a circle-shaped closed curve
#

radius = 35
numberOfControlPoints = 40
import math
circleCurveNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsClosedCurveNode")
for controlPointIndex in range(numberOfControlPoints):
    angle = 2.0 * math.pi * controlPointIndex / numberOfControlPoints
    pointIndex = circleCurveNode.AddControlPoint(vtk.vtkVector3d(radius * math.sin(angle), radius * math.cos(angle), 0.0))

# Visualize

updateInfo = createCoordinateSystemsModel(circleCurveNode, 2)
curveObservations.append(addCoordinateSystemUpdater(updateInfo))

# Check quantitative results

if not circleCurveNode.GetCurvePointToWorldTransformAtPointIndex(6, curvePointToWorldTransform):
    raise Exception("Test4. GetCurvePointToWorldTransformAtPointIndex failed")

curvePointToWorldMatrix = slicer.util.arrayFromVTKMatrix(curvePointToWorldTransform)
expectedCurvePointToWorldMatrix = np.array(
    [[0.10190135, 0., 0.99479451, 3.29378772],
     [0.99479451, 0., -0.10190135, 34.84461594],
        [0., 1., 0., 0.],
        [0., 0., 0., 1.]])

if not np.isclose(curvePointToWorldMatrix, expectedCurvePointToWorldMatrix).all():
    raise Exception(f"Test4. CurvePointToWorldTransformAtPointIndex value incorrect: got {curvePointToWorldMatrix}, expected {expectedCurvePointToWorldMatrix}.")

#
# Remove observations
#

# Uncomment the next line to remove observations (stop automatic update of coordinate system spikes)
# removeCoordinateSystemUpdaters(curveObservations)

print('Test completed')
