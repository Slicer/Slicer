# markups measurements test

import json
import os

import numpy as np
import vtk

import slicer

#
# Test markups measurements (except curves, which are tested in MarkupsCurveMeasurementsTest.py)
#

markupsMeasurementsTestDir = slicer.app.temporaryPath + '/markupsMeasurementsTest'
print('Test directory: ', markupsMeasurementsTestDir)
if not os.access(markupsMeasurementsTestDir, os.F_OK):
    os.mkdir(markupsMeasurementsTestDir)

preserveFiles = True

#
# Line
#

markupsNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsLineNode')
length = 34.12
direction = np.array([0.3, -0.4, 0.8])
pos1 = np.array([20, -12.4, 3.8])
pos2 = pos1 + direction / np.linalg.norm(direction) * length
markupsNode.AddControlPoint(vtk.vtkVector3d(pos1))
markupsNode.AddControlPoint(vtk.vtkVector3d(pos2))

measurement = markupsNode.GetMeasurement("length")
if abs(measurement.GetValue() - length) > 1e-4:
    raise Exception("Unexpected length value: " + str(measurement.GetValue()))

if measurement.GetValueWithUnitsAsPrintableString() != '34.12mm':
    raise Exception("Unexpected length measurement result: " + measurement.GetValueWithUnitsAsPrintableString())

markupsFilename = markupsMeasurementsTestDir + '/line.mkp.json'
slicer.util.saveNode(markupsNode, markupsFilename)

with open(markupsFilename) as f:
    markupsJson = json.load(f)

result = [{'name': 'length', 'enabled': True, 'value': 34.12, 'units': 'mm', 'printFormat': '%-#4.4gmm'}]
if markupsJson['markups'][0]['measurements'] != result:
    raise Exception("Unexpected length measurement result in file: " + str(markupsJson['markups'][0]['measurements']))

if not preserveFiles:
    os.remove(markupsFilename)

#
# Angle
#

markupsNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsAngleNode')
markupsNode.AddControlPoint(vtk.vtkVector3d(30, -22.4, 13.8))
markupsNode.AddControlPoint(vtk.vtkVector3d(12, -2.9, 22.2))
markupsNode.AddControlPoint(vtk.vtkVector3d(-5, 9.4, 1.8))

measurement = markupsNode.GetMeasurement("angle")
if abs(measurement.GetValue() - 117.4) > 0.1:
    raise Exception("Unexpected angle value: " + str(measurement.GetValue()))

if measurement.GetValueWithUnitsAsPrintableString() != '117.4deg':
    raise Exception("Unexpected angle measurement result: " + measurement.GetValueWithUnitsAsPrintableString())

markupsFilename = markupsMeasurementsTestDir + '/angle.mkp.json'
slicer.util.saveNode(markupsNode, markupsFilename)

with open(markupsFilename) as f:
    markupsJson = json.load(f)

result = [{'name': 'angle', 'enabled': True, 'value': 117.36891896165277, 'units': 'deg', 'printFormat': '%3.1f%s'}]
if markupsJson['markups'][0]['measurements'] != result:
    raise Exception("Unexpected angle measurement result in file: " + str(markupsJson['markups'][0]['measurements']))

markupsNode.SetAngleMeasurementModeToOrientedPositive()
measurement = markupsNode.GetMeasurement("angle")
if abs(measurement.GetValue() - 242.6) > 0.1:
    raise Exception("Unexpected angle value: " + str(measurement.GetValue()))

markupsNode.SetAngleMeasurementModeToOrientedSigned()
measurement = markupsNode.GetMeasurement("angle")
if abs(measurement.GetValue() - (-117.36891896165277)) > 0.1:
    raise Exception("Unexpected angle value: " + str(measurement.GetValue()))

if not preserveFiles:
    os.remove(markupsFilename)

#
# Plane
#

markupsNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsPlaneNode')
markupsNode.SetPlaneType(slicer.vtkMRMLMarkupsPlaneNode.PlaneType3Points)
markupsNode.AddControlPoint(vtk.vtkVector3d(30, -22.4, 13.8))
markupsNode.AddControlPoint(vtk.vtkVector3d(50, -22.4, 13.8))
markupsNode.AddControlPoint(vtk.vtkVector3d(30, -62.4, 13.8))

measurement = markupsNode.GetMeasurement("area")
measurement.SetEnabled(True)
if measurement.GetValueWithUnitsAsPrintableString() != '32.00cm2':
    raise Exception("Unexpected area measurement result: " + measurement.GetValueWithUnitsAsPrintableString())

markupsFilename = markupsMeasurementsTestDir + '/plane.mkp.json'
slicer.util.saveNode(markupsNode, markupsFilename)

with open(markupsFilename) as f:
    markupsJson = json.load(f)

result = [{'name': 'area', 'enabled': True, 'value': 32.00000000000001, 'units': 'cm2', 'printFormat': '%-#4.4gcm2'}]
if markupsJson['markups'][0]['measurements'] != result:
    raise Exception("Unexpected area measurement result in file: " + str(markupsJson['markups'][0]['measurements']))

if not preserveFiles:
    os.remove(markupsFilename)

#
# ROI
#

markupsNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsROINode')
markupsNode.AddControlPoint(vtk.vtkVector3d(61, -12.4, 13.8))
markupsNode.SetSize(20, 30, 40)

measurement = markupsNode.GetMeasurement("volume")
measurement.SetEnabled(True)
if measurement.GetValueWithUnitsAsPrintableString() != '24.000cm3':
    raise Exception("Unexpected volume measurement result: " + measurement.GetValueWithUnitsAsPrintableString())

markupsFilename = markupsMeasurementsTestDir + '/roi.mkp.json'
slicer.util.saveNode(markupsNode, markupsFilename)

with open(markupsFilename) as f:
    markupsJson = json.load(f)

result = [{'name': 'volume', 'enabled': True, 'value': 24.000000000000007, 'units': 'cm3', 'printFormat': '%-#4.5gcm3'}]
if markupsJson['markups'][0]['measurements'] != result:
    raise Exception("Unexpected volume measurement result in file: " + str(markupsJson['markups'][0]['measurements']))

if not preserveFiles:
    os.remove(markupsFilename)

print('Markups computation is verified successfully')
