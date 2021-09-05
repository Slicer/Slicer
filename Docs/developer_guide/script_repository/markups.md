## Markups

### Load markups fiducial list from file

Markups fiducials can be loaded from file:

```python
slicer.util.loadMarkupsFiducialList("/path/to/list/F.fcsv")
```

### Adding Fiducials Programmatically

Markups fiducials can be added to the currently active list from the python console by using the following module logic command:

```python
slicer.modules.markups.logic().AddFiducial()
```

The command with no arguments will place a new fiducial at the origin. You can also pass it an initial location:

```python
slicer.modules.markups.logic().AddFiducial(1.0, -2.0, 3.3)
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

### Add a button to module GUI to activate fiducial placement

This code snippet creates a toggle button, which activates fiducial placement when pressed (and deactivates when released).

The [qSlicerMarkupsPlaceWidget widget](http://apidocs.slicer.org/master/classqSlicerMarkupsPlaceWidget.html) can automatically activate placement of multiple points and can show buttons for deleting points, changing colors, lock, and hide points.

```python
w=slicer.qSlicerMarkupsPlaceWidget()
w.setMRMLScene(slicer.mrmlScene)
markupsNodeID = slicer.modules.markups.logic().AddNewFiducialNode()
w.setCurrentNode(slicer.mrmlScene.GetNodeByID(markupsNodeID))
# Hide all buttons and only show place button
w.buttonsVisible=False
w.placeButton().show()
w.show()
```

### Adding Fiducials via mouse clicks

You can also set the mouse mode into Markups fiducial placement by calling:

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

To switch back to view transform once you're done placing fiducials:

```python
interactionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLInteractionNodeSingleton")
interactionNode.SwitchToViewTransformMode()
# also turn off place mode persistence if required
interactionNode.SetPlaceModePersistence(0)
```

### Access to Fiducial Properties

Each vtkMRMLMarkupsFiducialNode has a vector of points in it which can be accessed from python:

```python
fidNode = getNode("vtkMRMLMarkupsFiducialNode1")
n = fidNode.AddFiducial(4.0, 5.5, -6.0)
fidNode.SetNthFiducialLabel(n, "new label")
# each markup is given a unique id which can be accessed from the superclass level
id1 = fidNode.GetNthMarkupID(n)
# manually set the position
fidNode.SetNthFiducialPosition(n, 6.0, 7.0, 8.0)
# set the label
fidNode.SetNthFiducialLabel(n, "New label")
# set the selected flag, only selected = 1 fiducials will be passed to CLIs
fidNode.SetNthFiducialSelected(n, 1)
# set the visibility flag
fidNode.SetNthFiducialVisibility(n, 0)
```

You can loop over the fiducials in a list and get the coordinates:

```python
fidList = slicer.util.getNode("F")
numFids = fidList.GetNumberOfFiducials()
for i in range(numFids):
  ras = [0,0,0]
  fidList.GetNthFiducialPosition(i,ras)
  # the world position is the RAS position with any transform matrices applied
  world = [0,0,0,0]
  fidList.GetNthFiducialWorldCoordinates(0,world)
  print(i,": RAS =",ras,", world =",world)
```

You can also look at the sample code in the [Endoscopy module](https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/Endoscopy/Endoscopy.py#L287) to see how python is used to access fiducials from a scripted module.

### Define/edit a circular region of interest in a slice viewer

Drop two markup points on a slice view and copy-paste the code below into the Python console. After this, as you move the markups you’ll see a circle following the markups.

```python
# Update the sphere from the fiducial points
def UpdateSphere(param1, param2):
  """Update the sphere from the fiducial points
  """
  import math
  centerPointCoord = [0.0, 0.0, 0.0]
  markups.GetNthFiducialPosition(0,centerPointCoord)
  circumferencePointCoord = [0.0, 0.0, 0.0]
  markups.GetNthFiducialPosition(1,circumferencePointCoord)
  sphere.SetCenter(centerPointCoord)
  radius=math.sqrt((centerPointCoord[0]-circumferencePointCoord[0])**2+(centerPointCoord[1]-circumferencePointCoord[1])**2+(centerPointCoord[2]-circumferencePointCoord[2])**2)
  sphere.SetRadius(radius)
  sphere.SetPhiResolution(30)
  sphere.SetThetaResolution(30)
  sphere.Update()

# Get markup node from scene
markups=slicer.util.getNode("F")
sphere = vtk.vtkSphereSource()
UpdateSphere(0,0)

# Create model node and add to scene
modelsLogic = slicer.modules.models.logic()
model = modelsLogic.AddModel(sphere.GetOutput())
model.GetDisplayNode().SetSliceIntersectionVisibility(True)
model.GetDisplayNode().SetSliceIntersectionThickness(3)
model.GetDisplayNode().SetColor(1,1,0)

# Call UpdateSphere whenever the fiducials are changed
markups.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, UpdateSphere, 2)
```

### Specify a sphere by multiple of markups points

Drop multiple markup points at the boundary of the spherical object and and copy-paste the code below into the Python console to get best-fit sphere. Minimum 4 points are required, it is recommended to place the points far from each other for most accurate fit.

```python
# Get markup node from scene
markups = slicer.util.getNode("F")

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

### Measure distances of points from a line

Draw a markups line (`L`) and drop markups fiducial points (`F`) in a view then run the following code snippet to compute distances of the points from the line.

```python
pointsNode = getNode("F")
lineNode = getNode("L")

# Get fiducial point positions and line endpoints as numpy arrays
points = slicer.util.arrayFromMarkupsControlPoints(pointsNode)
line = slicer.util.arrayFromMarkupsControlPoints(lineNode)
# Compute distance of points from the line
from numpy import cross
from numpy.linalg import norm
for i, point in enumerate(points):
    d = norm(cross(line[1]-line[0],point-line[0])/norm(line[1]-line[0]))
    print(f"Point {i}: Position = {point}. Distance from line = {d}.")
```

### Set slice position and orientation from 3 markup fiducials

Drop 3 markup points in the scene and copy-paste the code below into the Python console. After this, as you move the markups you’ll see the red slice view position and orientation will be set to make it fit to the 3 points.

```python
# Update plane from fiducial points
def UpdateSlicePlane(param1=None, param2=None):
  # Get point positions as numpy array
  import numpy as np
  nOfFiduciallPoints = markups.GetNumberOfFiducials()
  if nOfFiduciallPoints < 3:
    return  # not enough points
  points = np.zeros([3,nOfFiduciallPoints])
  for i in range(0, nOfFiduciallPoints):
    markups.GetNthFiducialPosition(i, points[:,i])
  # Compute plane position and normal
  planePosition = points.mean(axis=1)
  planeNormal = np.cross(points[:,1] - points[:,0], points[:,2] - points[:,0])
  planeX = points[:,1] - points[:,0]
  sliceNode.SetSliceToRASByNTP(planeNormal[0], planeNormal[1], planeNormal[2],
    planeX[0], planeX[1], planeX[2],
    planePosition[0], planePosition[1], planePosition[2], 0)

# Get markup node from scene
sliceNode = slicer.app.layoutManager().sliceWidget("Red").mrmlSliceNode()
markups = slicer.util.getNode("F")

# Update slice plane manually
UpdateSlicePlane()

# Update slice plane automatically whenever points are changed
markupObservation = [markups, markups.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, UpdateSlicePlane, 2)]
```

To stop automatic updates, run this:

```python
markupObservation[0].RemoveObserver(markupObservation[1])
```

### Switching to markup fiducial placement mode

To activate a fiducial placement mode, both interaction mode has to be set and a fiducial node has to be selected:

```python
interactionNode = slicer.app.applicationLogic().GetInteractionNode()
selectionNode = slicer.app.applicationLogic().GetSelectionNode()
selectionNode.SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode")
fiducialNode = slicer.vtkMRMLMarkupsFiducialNode()
slicer.mrmlScene.AddNode(fiducialNode)
fiducialNode.CreateDefaultDisplayNodes()
selectionNode.SetActivePlaceNodeID(fiducialNode.GetID())
interactionNode.SetCurrentInteractionMode(interactionNode.Place)
```

Alternatively, *qSlicerMarkupsPlaceWidget* widget can be used to initiate markup placement:

```python
# Temporary markups node
markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")

def placementModeChanged(active):
  print("Placement: " +("active" if active else "inactive"))
  # You can inspect what is in the markups node here, delete the temporary markup node, etc.

# Create and set up widget that contains a single "place markup" button. The widget can be placed in the module GUI.
placeWidget = slicer.qSlicerMarkupsPlaceWidget()
placeWidget.setMRMLScene(slicer.mrmlScene)
placeWidget.setCurrentNode(markupsNode)
placeWidget.buttonsVisible=False
placeWidget.placeButton().show()
placeWidget.connect("activeMarkupsFiducialPlaceModeChanged(bool)", placementModeChanged)
placeWidget.show()
```

### Change markup fiducial display properties

Display properties are stored in display node(s) associated with the fiducial node.

```python
fiducialNode = getNode("F")
fiducialDisplayNode = fiducialNode.GetDisplayNode()
fiducialDisplayNode.SetVisibility(False) # Hide all points
fiducialDisplayNode.SetVisibility(True) # Show all points
fiducialDisplayNode.SetSelectedColor(1,1,0) # Set color to yellow
fiducialDisplayNode.SetViewNodeIDs(["vtkMRMLSliceNodeRed", "vtkMRMLViewNode1"]) # Only show in red slice view and first 3D view
```

### Get a notification if a markup point position is modified

Event management of Slicer-4.11 version is still subject to change. The example below shows how point manipulation can be observed now.

```python
def onMarkupChanged(caller,event):
  markupsNode = caller
  sliceView = markupsNode.GetAttribute("Markups.MovingInSliceView")
  movingMarkupIndex = markupsNode.GetDisplayNode().GetActiveControlPoint()
  if movingMarkupIndex >= 0:
    pos = [0,0,0]
    markupsNode.GetNthFiducialPosition(movingMarkupIndex, pos)
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

markupsNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode")
markupsNode.CreateDefaultDisplayNodes()
markupsNode.AddFiducial(0,0,0)
markupsNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointModifiedEvent, onMarkupChanged)
markupsNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointStartInteractionEvent, onMarkupStartInteraction)
markupsNode.AddObserver(slicer.vtkMRMLMarkupsNode.PointEndInteractionEvent, onMarkupEndInteraction)
```

### Write markup positions to JSON file

```python
markupNode = getNode("F")
outputFileName = "c:/tmp/test.json"

# Get markup positions
data = []
for fidIndex in range(markupNode.GetNumberOfFiducials()):
  coords=[0,0,0]
  markupNode.GetNthFiducialPosition(fidIndex,coords)
  data.append({"label": markupNode.GetNthFiducialLabel(), "position": coords})

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

### Fit slice plane to markup fiducials

```python
sliceNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceNodeRed")
markupsNode = slicer.mrmlScene.GetFirstNodeByName("F")
# Get markup point positions as numpy arrays
import numpy as np
p1 = np.zeros(3)
p2 = np.zeros(3)
p3 = np.zeros(3)
markupsNode.GetNthFiducialPosition(0, p1)
markupsNode.GetNthFiducialPosition(1, p2)
markupsNode.GetNthFiducialPosition(2, p3)
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

The [qSlicerSimpleMarkupsWidget](http://apidocs.slicer.org/master/classqSlicerSimpleMarkupsWidget.html) can be integrated into module widgets to display list of markups control points and initiate placement. An example of this use is in [Gel Dosimetry module](https://www.slicer.org/wiki/Documentation/Nightly/Modules/GelDosimetry).

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
      volumeNode = slicer.mrmlScene.GetNodeByID(lineNode.GetNthMarkupAssociatedNodeID(0))
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
