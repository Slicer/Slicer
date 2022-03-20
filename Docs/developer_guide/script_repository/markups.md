## Markups

### Save markups to file

Any markup node can be saved as a [markups json file](modules/markups.md):

```python
markupsNode = slicer.util.getNode('F')
slicer.util.saveNode(markupsNode, "/path/to/MyMarkups.mkp.json")
```

Generally the markups json file format is recommended for saving all properties of a markups node, but for exporting only control point information (name, position, and basic state) a [control points table can be exported in standard csv file format](modules/markups.md#markups-control-points-table-file-format-csv-tsv):

```python
slicer.modules.markups.logic().ExportControlPointsToCSV(markupsNode, "/path/to/MyControlPoints.csv")
```

### Load markups from file

Any markup node can be loaded from a [markups json file](modules/markups.md):

```python
markupsNode = slicer.util.loadMarkups("/path/to/MyMarkups.mkp.json")
```

Control points can be loaded from [control points table csv file](modules/markups.md#markups-control-points-table-file-format-csv-tsv):

```python
markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsCurveNode")
slicer.modules.markups.logic().ImportControlPointsFromCSV(markupsNode, "/path/to/MyControlPoints.csv")
```

### Load markups point list from file

Markups point list can be loaded from legacy [fcsv file format](modules/markups.md#markups-fiducial-point-list-file-format-fcsv). Note that this file format is no longer recommended, as it is not a standard csv file format and can only store a small fraction of information that is in a markups node.

```python
slicer.util.loadMarkupsFiducialList("/path/to/list/F.fcsv")
```

### Adding control points Programmatically

Markups control points can be added to the currently active point list from the python console by using the following module logic command:

```python
slicer.modules.markups.logic().AddControlPoint()
```

The command with no arguments will place a new control point at the origin. You can also pass it an initial location:

```python
slicer.modules.markups.logic().AddControlPoint(1.0, -2.0, 3.3)
```

### How to draw a curve using control points stored in a numpy array

```python
# Create random numpy array to use as input
import numpy as np
pointPositions = np.random.uniform(-50,50,size=[15,3])

# Create curve from numpy array
curveNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsCurveNode")
slicer.util.updateMarkupsControlPointsFromArray(curveNode, pointPositions)
```

### Add a button to module GUI to activate control point placement

This code snippet creates a toggle button, which activates control point placement when pressed (and deactivates when released).

The [qSlicerMarkupsPlaceWidget widget](https://apidocs.slicer.org/master/classqSlicerMarkupsPlaceWidget.html) can automatically activate placement of multiple points and can show buttons for deleting points, changing colors, lock, and hide points.

```python
w=slicer.qSlicerMarkupsPlaceWidget()
w.setMRMLScene(slicer.mrmlScene)
markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsCurveNode")
w.setCurrentNode(slicer.mrmlScene.GetNodeByID(markupsNode.GetID()))
# Hide all buttons and only show place button
w.buttonsVisible=False
w.placeButton().show()
w.show()
```

### Adding control points via mouse clicks

You can also set the mouse mode into Markups control point placement by calling:

```python
placeModePersistence = 1
slicer.modules.markups.logic().StartPlaceMode(placeModePersistence)
```

A lower level way to do this is via the selection and interaction nodes:

```python
selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")
selectionNode.SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode")
interactionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLInteractionNodeSingleton")
placeModePersistence = 1
interactionNode.SetPlaceModePersistence(placeModePersistence)
# mode 1 is Place, can also be accessed via slicer.vtkMRMLInteractionNode().Place
interactionNode.SetCurrentInteractionMode(1)
```

To switch back to view transform once you're done placing control points:

```python
interactionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLInteractionNodeSingleton")
interactionNode.SwitchToViewTransformMode()
# also turn off place mode persistence if required
interactionNode.SetPlaceModePersistence(0)
```

### Access to markups point list Properties

Each vtkMRMLMarkupsFiducialNode has a vector of control points in it which can be accessed from python:

```python
pointListNode = getNode("vtkMRMLMarkupsFiducialNode1")
n = pointListNode.AddControlPoint([4.0, 5.5, -6.0])
pointListNode.SetNthControlPointLabel(n, "new label")
# each control point is given a unique id which can be accessed from the superclass level
id1 = pointListNode.GetNthControlPointID(n)
# manually set the position
pointListNode.SetNthControlPointPosition(n, 6.0, 7.0, 8.0)
# set the label
pointListNode.SetNthControlPointLabel(n, "New label")
# set the selected flag, only selected = 1 control points will be passed to CLIs
pointListNode.SetNthControlPointSelected(n, 1)
# set the visibility flag
pointListNode.SetNthControlPointVisibility(n, 0)
```

You can loop over the control points in a list and get the coordinates:

```python
pointListNode = slicer.util.getNode("F")
numControlPoints = pointListNode.GetNumberOfControlPoints()
for i in range(numControlPoints):
  ras = vtk.vtkVector3d(0,0,0)
  pointListNode.GetNthControlPointPosition(i,ras)
  # the world position is the RAS position with any transform matrices applied
  world = [0.0, 0.0, 0.0]
  pointListNode.GetNthControlPointPositionWorld(i,world)
  print(i,": RAS =",ras,", world =",world)
```

You can also look at the sample code in the [Endoscopy module](https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/Endoscopy/Endoscopy.py#L287) to see how python is used to access control points from a scripted module.

### Define/edit a circular region of interest in a slice viewer

Drop two markups control points on a slice view and copy-paste the code below into the Python console. After this, as you move the control points you’ll see a circle following the markups.

```python
# Update the sphere from the control points
def UpdateSphere(param1, param2):
  """Update the sphere from the control points
  """
  import math
  pointListNode = slicer.util.getNode("F")
  centerPointCoord = [0.0, 0.0, 0.0]
  pointListNode.GetNthControlPointPosition(0,centerPointCoord)
  circumferencePointCoord = [0.0, 0.0, 0.0]
  pointListNode.GetNthControlPointPosition(1,circumferencePointCoord)
  sphere.SetCenter(centerPointCoord)
  radius=math.sqrt((centerPointCoord[0]-circumferencePointCoord[0])**2+(centerPointCoord[1]-circumferencePointCoord[1])**2+(centerPointCoord[2]-circumferencePointCoord[2])**2)
  sphere.SetRadius(radius)
  sphere.SetPhiResolution(30)
  sphere.SetThetaResolution(30)
  sphere.Update()

# Get point list node from scene
pointListNode = slicer.util.getNode("F")
sphere = vtk.vtkSphereSource()
UpdateSphere(0,0)

# Create model node and add to scene
modelsLogic = slicer.modules.models.logic()
model = modelsLogic.AddModel(sphere.GetOutput())
model.GetDisplayNode().SetSliceIntersectionVisibility(True)
model.GetDisplayNode().SetSliceIntersectionThickness(3)
model.GetDisplayNode().SetColor(1,1,0)

# Call UpdateSphere whenever the control points are changed
pointListNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, UpdateSphere, 2)
```

### Specify a sphere by multiple control points

Drop multiple markups control points at the boundary of the spherical object and and copy-paste the code below into the Python console to get best-fit sphere. A minimum of 4 control points are required. Tt is recommended to place the control points far away from each other for the most accurate fit.

```python
# Get markup node from scene
pointListNode = slicer.util.getNode("F")

from scipy.optimize import least_squares
import numpy

def fit_sphere_least_squares(x_values, y_values, z_values, initial_parameters, bounds=((-numpy.inf, -numpy.inf, -numpy.inf, -numpy.inf),(numpy.inf, numpy.inf, numpy.inf, numpy.inf))):
  """
  Source: https://github.com/thompson318/scikit-surgery-sphere-fitting/blob/master/sksurgeryspherefitting/algorithms/sphere_fitting.py
  Uses scipy's least squares optimisor to fit a sphere to a set
  of 3D Points
  :return: x: an array containing the four fitted parameters
  :return: ier: int An integer flag. If it is equal to 1, 2, 3 or 4, the
          solution was found.
  :param: (x,y,z) three arrays of equal length containing the x, y, and z
          coordinates.
  :param: an array containing four initial values (centre, and radius)
  """
  return least_squares(_calculate_residual_sphere, initial_parameters, bounds=bounds, method="trf", jac="3-point", args=(x_values, y_values, z_values))

def _calculate_residual_sphere(parameters, x_values, y_values, z_values):
  """
  Source: https://github.com/thompson318/scikit-surgery-sphere-fitting/blob/master/sksurgeryspherefitting/algorithms/sphere_fitting.py
  Calculates the residual error for an x,y,z coordinates, fitted
  to a sphere with centre and radius defined by the parameters tuple
  :return: The residual error
  :param: A tuple of the parameters to be optimised, should contain [x_centre, y_centre, z_centre, radius]
  :param: arrays containing the x,y, and z coordinates.
  """
  #extract the parameters
  x_centre, y_centre, z_centre, radius = parameters
  #use numpy's sqrt function here, which works by element on arrays
  distance_from_centre = numpy.sqrt((x_values - x_centre)**2 + (y_values - y_centre)**2 + (z_values - z_centre)**2)
  return distance_from_centre - radius

# Fit a sphere to the markups fidicual points
markupsPositions = slicer.util.arrayFromMarkupsControlPoints(markups)
import numpy as np
# initial guess
center0 = np.mean(markupsPositions, 0)
radius0 = np.linalg.norm(np.amin(markupsPositions,0)-np.amax(markupsPositions,0))/2.0
fittingResult = fit_sphere_least_squares(markupsPositions[:,0], markupsPositions[:,1], markupsPositions[:,2], [center0[0], center0[1], center0[2], radius0])
[centerX, centerY, centerZ, radius] = fittingResult["x"]

# Create a sphere using the fitted parameters
sphere = vtk.vtkSphereSource()
sphere.SetPhiResolution(30)
sphere.SetThetaResolution(30)
sphere.SetCenter(centerX, centerY, centerZ)
sphere.SetRadius(radius)
sphere.Update()

# Add the sphere to the scene
modelsLogic = slicer.modules.models.logic()
model = modelsLogic.AddModel(sphere.GetOutput())
model.GetDisplayNode().SetSliceIntersectionVisibility(True)
model.GetDisplayNode().SetSliceIntersectionThickness(3)
model.GetDisplayNode().SetColor(1,1,0)
```

### Fit markups ROI to volume

This code snippet creates a new markups ROI and fits it to a volume node.

```python
volumeNode = getNode('MRHead')

# Create a new ROI that will be fit to volumeNode
roiNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsROINode")

cropVolumeParameters = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLCropVolumeParametersNode")
cropVolumeParameters.SetInputVolumeNodeID(volumeNode.GetID())
cropVolumeParameters.SetROINodeID(roiNode.GetID())
slicer.modules.cropvolume.logic().SnapROIToVoxelGrid(cropVolumeParameters)  # optional (rotates the ROI to match the volume axis directions)
slicer.modules.cropvolume.logic().FitROIToInputVolume(cropVolumeParameters)
slicer.mrmlScene.RemoveNode(cropVolumeParameters)
```

### Fit markups plane to model

This code snippet fits a plane a model node named `InputModel` and creates a new markups plane node to display this best fit plane.

```python
inputModel = getNode('InputModel')

# Compute best fit plane
center = [0.0, 0.0, 0.0]
normal = [0.0, 0.0, 1.0]
vtk.vtkPlane.ComputeBestFittingPlane(inputModel.GetPolyData().GetPoints(), center, normal)

# Display best fit plane as a markups plane
planeNode = slicer.mrmlScene.AddNewNodeByClass('vtkMRMLMarkupsPlaneNode')
planeNode.SetCenter(center)
planeNode.SetNormal(normal)
```

### Measure angle between two markup planes

Measure angle between two markup plane nodes. Whenever any of the plane nodes are moved, the updated angle is printed on the console.

```python
planeNodeNames = ["P", "P_1"]

# Print angles between slice nodes
def ShowAngle(unused1=None, unused2=None):
  planeNormalVectors = []
  for planeNodeName in planeNodeNames:
    planeNode = slicer.util.getFirstNodeByClassByName("vtkMRMLMarkupsPlaneNode", planeNodeName)
    planeNormalVector = [0.0, 0.0, 0.0]
    planeNode.GetNormalWorld(planeNormalVector)
    planeNormalVectors.append(planeNormalVector)
  angleRad = vtk.vtkMath.AngleBetweenVectors(planeNormalVectors[0], planeNormalVectors[1])
  angleDeg = vtk.vtkMath.DegreesFromRadians(angleRad)
  print("Angle between planes {0} and {1} = {2:0.3f}".format(planeNodeNames[0], planeNodeNames[1], angleDeg))

# Observe plane node changes
for planeNodeName in planeNodeNames:
  planeNode = slicer.util.getFirstNodeByClassByName("vtkMRMLMarkupsPlaneNode", planeNodeName)
  planeNode.AddObserver(slicer.vtkMRMLMarkupsPlaneNode.PointModifiedEvent, ShowAngle)

# Print current angle
ShowAngle()
```

### Measure angle between two markup lines

Measure angle between two markup line nodes that are already added to the scene and have the names `L` and `L_1`. Whenever either line is moved, the updated angle is printed on the console. This is for illustration only, for standard angle measurements angle markups can be used.

```python
lineNodeNames = ["L", "L_1"]

# Print angles between slice nodes
def ShowAngle(unused1=None, unused2=None):
  import numpy as np
  lineDirectionVectors = []
  for lineNodeName in lineNodeNames:
    lineNode = slicer.util.getFirstNodeByClassByName("vtkMRMLMarkupsLineNode", lineNodeName)
    lineStartPos = np.zeros(3)
    lineEndPos = np.zeros(3)
    lineNode.GetNthControlPointPositionWorld(0, lineStartPos)
    lineNode.GetNthControlPointPositionWorld(1, lineEndPos)
    lineDirectionVector = (lineEndPos-lineStartPos)/np.linalg.norm(lineEndPos-lineStartPos)
    lineDirectionVectors.append(lineDirectionVector)
  angleRad = vtk.vtkMath.AngleBetweenVectors(lineDirectionVectors[0], lineDirectionVectors[1])
  angleDeg = vtk.vtkMath.DegreesFromRadians(angleRad)
  print("Angle between lines {0} and {1} = {2:0.3f}".format(lineNodeNames[0], lineNodeNames[1], angleDeg))

# Observe line node changes
for lineNodeName in lineNodeNames:
  lineNode = slicer.util.getFirstNodeByClassByName("vtkMRMLMarkupsLineNode", lineNodeName)
  lineNode.AddObserver(slicer.vtkMRMLMarkupsLineNode.PointModifiedEvent, ShowAngle)

# Print current angle
ShowAngle()
```

### Project a line to a plane

Create a new line (`projectedLineNode`) by projecting a line (`lineNode`) to a plane (`planeNode`).

Each control point is projected by computing coordinates in the plane coordinate system, zeroing the z coordinate (distance from plane) then transforming  back the coordinates to the world coordinate system.

Transformation require homogeneous coordinates (1.0 appended to the 3D position), therefore 1.0 is added to the position after getting from the line and the 1.0 is removed when the computed point is added to the output line.

```python
lineNode = getNode('L')
planeNode = getNode('P')

# Create new node for storing the projected line node
projectedLineNode = slicer.mrmlScene.AddNewNodeByClass(lineNode.GetClassName(), lineNode.GetName()+" projected")

# Get transforms
planeToWorld = vtk.vtkMatrix4x4()
planeNode.GetObjectToWorldMatrix(planeToWorld)
worldToPlane = vtk.vtkMatrix4x4()
vtk.vtkMatrix4x4.Invert(planeToWorld, worldToPlane)

# Project each point
for pointIndex in range(2):
    point_World = [*lineNode.GetNthControlPointPositionWorld(pointIndex), 1.0]
    point_Plane = worldToPlane.MultiplyPoint(point_World)
    projectedPoint_Plane = [point_Plane[0], point_Plane[1], 0.0, 1.0]
    projectedPoint_World = planeToWorld.MultiplyPoint(projectedPoint_Plane)
    projectedLineNode.AddControlPoint(projectedPoint_World[0:3])
```

### Measure distances of points from a line

Draw a markups line (`L`) and drop markups point list (`F`) in a view and then run the following code snippet to compute distances of the points from the line.

```python
pointListNode = getNode("F")
lineNode = getNode("L")

# Get point list control point positions and line endpoints as numpy arrays
points = slicer.util.arrayFromMarkupsControlPoints(pointListNode)
line = slicer.util.arrayFromMarkupsControlPoints(lineNode)
# Compute distance of control points from the line
from numpy import cross
from numpy.linalg import norm
for i, point in enumerate(points):
    d = norm(cross(line[1]-line[0],point-line[0])/norm(line[1]-line[0]))
    print(f"Point {i}: Position = {point}. Distance from line = {d}.")
```

### Set slice position and orientation from 3 markups control points

Drop 3 markups control points in the scene and copy-paste the code below into the Python console. After this, as you move the control points you’ll see the red slice view position and orientation will be set to make it fit to the 3 points.

```python
# Update plane from control points
def UpdateSlicePlane(param1=None, param2=None):
  # Get control point positions as numpy array
  import numpy as np
  nOfControlPoints = pointListNode.GetNumberOfControlPoints()
  if nOfControlPoints < 3:
    return  # not enough control points
  points = np.zeros([3,nOfControlPoints])
  for i in range(0, nOfControlPoints):
    pointListNode.GetNthControlPointPosition(i, points[:,i])
  # Compute plane position and normal
  planePosition = points.mean(axis=1)
  planeNormal = np.cross(points[:,1] - points[:,0], points[:,2] - points[:,0])
  planeX = points[:,1] - points[:,0]
  sliceNode.SetSliceToRASByNTP(planeNormal[0], planeNormal[1], planeNormal[2],
    planeX[0], planeX[1], planeX[2],
    planePosition[0], planePosition[1], planePosition[2], 0)

# Get point list node from scene
sliceNode = slicer.app.layoutManager().sliceWidget("Red").mrmlSliceNode()
pointListNode = slicer.util.getNode("F")

# Update slice plane manually
UpdateSlicePlane()

# Update slice plane automatically whenever points are changed
pointListObservation = [pointListNode, pointListNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, UpdateSlicePlane, 2)]
```

To stop automatic updates, run this:

```python
pointListObservation[0].RemoveObserver(pointListObservation[1])
```

### Switching to markups control point placement mode

To activate control point placement mode for a point list, both interaction mode has to be set and a point list node has to be selected:

```python
interactionNode = slicer.app.applicationLogic().GetInteractionNode()
selectionNode = slicer.app.applicationLogic().GetSelectionNode()
selectionNode.SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode")
pointListNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
selectionNode.SetActivePlaceNodeID(pointListNode.GetID())
interactionNode.SetCurrentInteractionMode(interactionNode.Place)
```

Alternatively, *qSlicerMarkupsPlaceWidget* widget can be used to initiate markup placement:

```python
# Temporary markups point list node
pointListNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")

def placementModeChanged(active):
  print("Placement: " +("active" if active else "inactive"))
  # You can inspect what is in the markups node here, delete the temporary markup point list node, etc.

# Create and set up widget that contains a single "place control point" button. The widget can be placed in the module GUI.
placeWidget = slicer.qSlicerMarkupsPlaceWidget()
placeWidget.setMRMLScene(slicer.mrmlScene)
placeWidget.setCurrentNode(pointListNode)
placeWidget.buttonsVisible=False
placeWidget.placeButton().show()
placeWidget.connect("activeMarkupsFiducialPlaceModeChanged(bool)", placementModeChanged)
placeWidget.show()
```

### Change markup point list display properties

Display properties are stored in display node(s) associated with the point list node.

```python
pointListNode = getNode("F")
pointListDisplayNode = pointListNode.GetDisplayNode()
pointListDisplayNode.SetVisibility(False) # Hide all points
pointListDisplayNode.SetVisibility(True) # Show all points
pointListDisplayNode.SetSelectedColor(1,1,0) # Set color to yellow
pointListDisplayNode.SetViewNodeIDs(["vtkMRMLSliceNodeRed", "vtkMRMLViewNode1"]) # Only show in red slice view and first 3D view
```

### Get a notification if a markup control point position is modified

Event management of Slicer-4.11 version is still subject to change. The example below shows how control point manipulation can be observed now.

```python
def onMarkupChanged(caller,event):
  markupsNode = caller
  sliceView = markupsNode.GetAttribute("Markups.MovingInSliceView")
  movingMarkupIndex = markupsNode.GetDisplayNode().GetActiveControlPoint()
  if movingMarkupIndex >= 0:
    pos = [0,0,0]
    markupsNode.GetNthControlPointPosition(movingMarkupIndex, pos)
    isPreview = markupsNode.GetNthControlPointPositionStatus(movingMarkupIndex) == slicer.vtkMRMLMarkupsNode.PositionPreview
    if isPreview:
      logging.info("Point {0} is previewed at {1} in slice view {2}".format(movingMarkupIndex, pos, sliceView))
    else:
      logging.info("Point {0} was moved {1} in slice view {2}".format(movingMarkupIndex, pos, sliceView))
  else:
    logging.info("Points modified: slice view = {0}".format(sliceView))

def onMarkupStartInteraction(caller, event):
  markupsNode = caller
  sliceView = markupsNode.GetAttribute("Markups.MovingInSliceView")
  movingMarkupIndex = markupsNode.GetDisplayNode().GetActiveControlPoint()
  logging.info("Start interaction: point ID = {0}, slice view = {1}".format(movingMarkupIndex, sliceView))

def onMarkupEndInteraction(caller, event):
  markupsNode = caller
  sliceView = markupsNode.GetAttribute("Markups.MovingInSliceView")
  movingMarkupIndex = markupsNode.GetDisplayNode().GetActiveControlPoint()
  logging.info("End interaction: point ID = {0}, slice view = {1}".format(movingMarkupIndex, sliceView))

pointListNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
pointListNode.AddControlPoint([0,0,0])
pointListNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, onMarkupChanged)
pointListNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointStartInteractionEvent, onMarkupStartInteraction)
pointListNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointEndInteractionEvent, onMarkupEndInteraction)
```

### Write markup control point positions to JSON file

```python
pointListNode = getNode("F")
outputFileName = "c:/tmp/test.json"

# Get markup positions
data = []
for fidIndex in range(pointListNode.GetNumberOfControlPoints()):
  coords=[0,0,0]
  pointListNode.GetNthControlPointPosition(fidIndex,coords)
  data.append({"label": pointListNode.GetNthControlPointLabel(), "position": coords})

import json
with open(outputFileName, "w") as outfile:
  json.dump(data, outfile)
```

### Write annotation ROI to JSON file

```python
roiNode = getNode("R")
outputFileName = "c:/tmp/test.json"

# Get annotation ROI data
center = [0,0,0]
radius = [0,0,0]
roiNode.GetControlPointWorldCoordinates(0, center)
roiNode.GetControlPointWorldCoordinates(1, radius)
data = {"center": radius, "radius": radius}

# Write to json file
import json
with open(outputFileName, "w") as outfile:
  json.dump(data, outfile)
```

### Fit slice plane to markup control points

```python
sliceNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeRed")
pointListNode = slicer.mrmlScene.GetFirstNodeByName("F")
# Get markup point positions as numpy arrays
import numpy as np
p1 = np.zeros(3)
p2 = np.zeros(3)
p3 = np.zeros(3)
pointListNode.GetNthControlPointPosition(0, p1)
pointListNode.GetNthControlPointPosition(1, p2)
pointListNode.GetNthControlPointPosition(2, p3)
# Get plane axis directions
n = np.cross(p2-p1, p2-p3) # plane normal direction
n = n/np.linalg.norm(n)
t = np.cross([0.0, 0.0, 1], n) # plane transverse direction
t = t/np.linalg.norm(t)
# Set slice plane orientation and position
sliceNode.SetSliceToRASByNTP(n[0], n[1], n[2], t[0], t[1], t[2], p1[0], p1[1], p1[2], 0)
```

### Change color of a markups node

Markups have `Color` and `SelectedColor` properties. `SelectedColor` is used if all control points are in "selected" state, which is the default. So, in most cases `SetSelectedColor` method must be used to change markups node color.

### Display list of control points in my module's GUI

The [qSlicerSimpleMarkupsWidget](https://apidocs.slicer.org/master/classqSlicerSimpleMarkupsWidget.html) can be integrated into module widgets to display list of markups control points and initiate placement. An example of this use is in [Gel Dosimetry module](https://www.slicer.org/wiki/Documentation/Nightly/Modules/GelDosimetry).

### Pre-populate the scene with measurements

This code snippet creates a set of predefined line markups (named A, B, C, D) in the scene when the user hits <kbd>Ctrl+N</kbd>.
How to use this:

1. Customize the code (replace A, B, C, D with your measurement names) and copy-paste the code into the Python console. This has to be done only once after Slicer is started. Add it to [.slicerrc.py file](../user_guide/settings.md#application-startup-file) so that it persists even if Slicer is restarted.
2. Load the data set that has to be measured
3. Hit Ctrl+N to create all the measurements
4. Go to Markups module to see the list of measurements
5. For each measurement: select it in the data tree, click on the place button on the toolbar then click in slice or 3D views

```python
sliceNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeRed")
def createMeasurements():
  for nodeName in ['A', 'B', 'C', 'D']:
    lineNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode", nodeName)
    lineNode.CreateDefaultDisplayNodes()
    dn = lineNode.GetDisplayNode()
    # Use crosshair glyph to allow more accurate point placement
    dn.SetGlyphTypeFromString("CrossDot2D")
    # Hide measurement result while markup up
    lineNode.GetMeasurement('length').SetEnabled(False)

shortcut1 = qt.QShortcut(slicer.util.mainWindow())
shortcut1.setKey(qt.QKeySequence("Ctrl+n"))
shortcut1.connect( 'activated()', createMeasurements)
```

### Copy all measurements in the scene to Excel

This code snippet creates a set of predefined line markups (named A, B, C, D) in the scene when the user hits Ctrl+N.
How to use this:

1. Copy-paste the code into the Python console. This has to be done only once after Slicer is started. Add it to [.slicerrc.py file](../user_guide/settings.md#application-startup-file) so that it persists even if Slicer is restarted.
2. Load the data set that has to be measured and place line markups (you can use the "Pre-populate the scene with measurements" script above to help with this)
3. Hit Ctrl+M to copy all line measurents to the clipboard
4. Switch to Excel and hit Ctrl+V to paste the results there
5. Save the scene, just in case later you need to review your measurements

```python
def copyLineMeasurementsToClipboard():
  measurements = []
  # Collect all line measurements from the scene
  lineNodes = getNodesByClass('vtkMRMLMarkupsLineNode')
  for lineNode in lineNodes:
    # Get node filename that the length was measured on
    try:
      volumeNode = slicer.mrmlScene.GetNodeByID(lineNode.GetNthControlPointAssociatedNodeID(0))
      imagePath = volumeNode.GetStorageNode().GetFileName()
    except:
      imagePath = ''
    # Get line node n
    measurementName = lineNode.GetName()
    # Get length measurement
    lineNode.GetMeasurement('length').SetEnabled(True)
    length = str(lineNode.GetMeasurement('length').GetValue())
    # Add fields to results
    measurements.append('\t'.join([imagePath, measurementName, length]))
  # Copy all measurements to clipboard (to be pasted into Excel)
  slicer.app.clipboard().setText("\n".join(measurements))
  slicer.util.delayDisplay(f"Copied {len(measurements)} length measurements to the clipboard.")

shortcut2 = qt.QShortcut(slicer.util.mainWindow())
shortcut2.setKey(qt.QKeySequence("Ctrl+m"))
shortcut2.connect( 'activated()', copyLineMeasurementsToClipboard)
```

### Use markups json files in Python - outside Slicer

The examples below show how to use markups json files outside Slicer, in any Python environment.

To access content of a json file it can be either read as a json document or directly into a [pandas](https://pandas.pydata.org/) dataframe using a single command.

#### Get a table of control point labels and positions

Get table from the first markups node in the file:

```python
import pandas as pd
controlPointsTable = pd.DataFrame.from_dict(pd.read_json(input_json_filename)['markups'][0]['controlPoints'])
```

Result:

```python
>>> controlPointsTable
  label                                        position
0   F-1  [-53.388409961685824, -73.33572796934868, 0.0]
1   F-2     [49.8682950191571, -88.58955938697324, 0.0]
2   F-3   [-25.22749042145594, 59.255268199233726, 0.0]
```

#### Access position of control points positions in separate x, y, z columns

```python
controlPointsTable[['x','y','z']] = pd.DataFrame(controlPointsTable['position'].to_list())
del controlPointsTable['position']
```

#### Write control points to a csv file

```python
controlPointsTable.to_csv(output_csv_filename)
```

Resulting csv file:

```python
   ,label,x,y,z
   0,F-1,-53.388409961685824,-73.33572796934868,0.0
   1,F-2,49.8682950191571,-88.58955938697324,0.0
   2,F-3,-25.22749042145594,59.255268199233726,0.0
```

### Assign custom actions to markups

Custom actions can be assigned to markups, which can be triggered by any interaction event (mouse or keyboard action). The actions can be detected by adding observers to the markup node's display node.

```python
# This example adds an action to the default double-click action on a markup
# and defines two new custom actions. It is done for all existing markups in the first 3D view.
#
# How to use:
# 1. Create markups nodes.
# 2. Run the script below.
# 3. Double-click on the markup -> this triggers toggleLabelVisibilty.
# 4. Hover the mouse over a markup then pressing `q` and `w` keys -> this triggers shrinkControlPoints and growControlPoints.

threeDViewWidget = slicer.app.layoutManager().threeDWidget(0)
markupsDisplayableManager = threeDViewWidget.threeDView().displayableManagerByClassName('vtkMRMLMarkupsDisplayableManager')

def shrinkControlPoints(caller, eventId):
  markupsDisplayNode = caller
  markupsDisplayNode.SetGlyphScale(markupsDisplayNode.GetGlyphScale()/1.1)

def growControlPoints(caller, eventId):
  markupsDisplayNode = caller
  markupsDisplayNode.SetGlyphScale(markupsDisplayNode.GetGlyphScale()*1.1)

def toggleLabelVisibility(caller, eventId):
  markupsDisplayNode = caller
  markupsDisplayNode.SetPointLabelsVisibility(not markupsDisplayNode.GetPointLabelsVisibility())

observations = []  # store the observations so that later can be removed
markupsDisplayNodes = slicer.util.getNodesByClass("vtkMRMLMarkupsDisplayNode")
for markupsDisplayNode in markupsDisplayNodes:
  # Assign keyboard shortcut to trigger custom actions
  markupsWidget = markupsDisplayableManager.GetWidget(markupsDisplayNode)
  # Left double-click interaction event is translated to markupsWidget.WidgetEventAction by default,
  # therefore we don't need to add an event translation for that. We just add two keyboard event translation for two custom actions
  markupsWidget.SetKeyboardEventTranslation(markupsWidget.WidgetStateOnWidget, vtk.vtkEvent.NoModifier, '\0', 0, "q", markupsWidget.WidgetEventCustomAction1)
  markupsWidget.SetKeyboardEventTranslation(markupsWidget.WidgetStateOnWidget, vtk.vtkEvent.NoModifier, '\0', 0, "w", markupsWidget.WidgetEventCustomAction2)
  # Add observer to custom actions
  observations.append([markupsDisplayNode, markupsDisplayNode.AddObserver(markupsDisplayNode.ActionEvent, toggleLabelVisibility)])
  observations.append([markupsDisplayNode, markupsDisplayNode.AddObserver(markupsDisplayNode.CustomActionEvent1, shrinkControlPoints)])
  observations.append([markupsDisplayNode, markupsDisplayNode.AddObserver(markupsDisplayNode.CustomActionEvent2, growControlPoints)])

# Remove observations when custom actions are not needed anymore by uncommenting these lines:
for observedNode, observation in observations:
  observedNode.RemoveObserver(observation)
```
