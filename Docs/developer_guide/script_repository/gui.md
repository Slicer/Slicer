## Install Slicer

There are different approaches to install Slicer and extensions programmatically:

- Install Slicer manually and install extensions by using `slicer.app.extensionsManagerModel()`. See example [below](script_repository.md#download-and-install-extension) and in [install-slicer-extension.py](https://github.com/pieper/SlicerDockers/blob/master/slicer-plus/install-slicer-extension.py)
- Directly interact with the REST API endpoints of https://slicer-packages.kitware.com using `curl` and `jq`. See [slicer-download.sh](https://github.com/Slicer/SlicerDocker/blob/master/scripts/slicer-download.sh)

## Launch Slicer

### Open a file with Slicer at the command line

Open Slicer to view the `c:\some\folder\MRHead.nrrd` image file:

```console
"c:\Users\myusername\AppData\Local\NA-MIC\Slicer 4.11.20210226\Slicer.exe" c:\some\folder\MRHead.nrrd
```

:::{note}

It is necessary to specify full path to the Slicer executable and to the file that needs to be loaded.

:::

On Windows, the installer registers the `Slicer` application during install. This makes it possible to use `start` command to launch the most recently installed Slicer. For example, this command on the command-line starts Slicer and loads an image:

```console
start Slicer c:\some\folder\MRHead.nrrd
```

To load a file with non-default options, you can use `--python-code` option to run `slicer.util.load...` commands.

### Open an .mrb file with Slicer at the command line

```console
Slicer.exe --python-code "slicer.util.loadScene('f:/2013-08-23-Scene.mrb')"
```

### Run Python commands in the Slicer environment

Run Python commands, without showing any graphical user interface:

```console
Slicer.exe --python-code "doSomething; doSomethingElse; etc." --testing --no-splash --no-main-window
```

Slicer exits when the commands are completed because `--testing` options is specified.

### Run a Python script file in the Slicer environment

Run a Python script (stored in script file), without showing any graphical user interface:

```console
Slicer.exe --python-script "/full/path/to/myscript.py" --no-splash --no-main-window
```

To make Slicer exit when the script execution is completed, call `sys.exit(errorCode)` (where `errorCode` is set 0 for success and other value to indicate error).

### Launch Slicer directly from a web browser

Slicer can be associated with the `slicer:` custom URL protocol in the operating system or web browser. This is done automatically in the Windows installer and can be set up on other operating systems manually. After this when the user clicks on a `slicer://...` URL in the web browser, Slicer will start and the `slicer.app` object emits a signal with the URL that modules can process. The DICOM module processes DICOMweb URLs, but any module can specify additional actions.

For example, [this module](https://gist.github.com/lassoan/534298ee567000a37b3cb89de01750b8) will download and view any image if the user clicks on an URL like this in the web browser:

```
slicer://viewer/?download=https%3A%2F%2Fgithub.com%2Frbumm%2FSlicerLungCTAnalyzer%2Freleases%2Fdownload%2FSampleData%2FLungCTAnalyzerChestCT.nrrd
```

For reference, the DICOM module downloads a study from a DICOMweb server and shows it when providing a URL like this (which is used for example in the [Kheops DICOM data sharing platform](https://kheops.online/)):

```
slicer://viewer/?studyUID=2.16.840.1.113669.632.20.121711.10000158860
    &access_token=k0zR6WAPpNbVguQ8gGUHp6
    &dicomweb_endpoint=http%3A%2F%2Fdemo.kheops.online%2Fapi
    &dicomweb_uri_endpoint=%20http%3A%2F%2Fdemo.kheops.online%2Fapi%2Fwado
```

#### MRML scene

### Get MRML node from the scene

Get markups point list node named `F` (useful for quickly getting access to a MRML node in the Python console):

```python
pointListNode = getNode('F')
# do something with the node... let's remove the first control point in it
pointListNode.RemoveNthControlPoint(0)
```

Getting the first volume node without knowing its name (useful if there is only one volume loaded):

```python
volumeNode = slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLScalarVolumeNode")
# do something with the node... let's change its display window/level
volumeNode.GetDisplayNode().SetAutoWindowLevel(False)
volumeNode.GetDisplayNode().SetWindowLevelMinMax(100, 200)
```

:::{note}

- {func}`slicer.util.getNode()` is recommended **only for interactive debugging** in the Python console/Jupyter notebook
  -  its input is intentionally defined vaguely (it can be either node ID or name and you can use wildcards such as `*`), which is good because it make it simpler to use, but the uncertain behavior is not good for general-purpose use in a module
  -  throws an exception so that the developer knows immediately that there was a typo or other unexpected error
- `slicer.mrmlScene.GetNodeByID()` is more appropriate when a module needs to access a MRML node:
  - its behavior is more predictable: it only accepts node ID as input. `slicer.mrmlScene.GetFirstNodeByName()` can be used to get a node by its name, but since multiple nodes in the scene can have the same name, it is not recommended to keep reference to a node by its name. Since node IDs may change when a scene is saved and reloaded, node ID should not be stored persistently, but [node references](mrml_overview.md#mrml-node-references) must be used instead
  - if node is not found it returns `None` (instead of throwing an exception), because this is often not considered an error in module code (it is just used to check existence of a node) and using return value for not-found nodes allows simpler syntax

:::

### Clone a node

This example shows how to make a copy of any node that appears in Subject Hierarchy (in Data module).

```python
# Get a node from SampleData that we will clone
import SampleData
nodeToClone = SampleData.SampleDataLogic().downloadMRHead()

# Clone the node
shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
itemIDToClone = shNode.GetItemByDataNode(nodeToClone)
clonedItemID = slicer.modules.subjecthierarchy.logic().CloneSubjectHierarchyItem(shNode, itemIDToClone)
clonedNode = shNode.GetItemDataNode(clonedItemID)
```

### Save a node to file

Save a transform node to file (should work with any other node type, if file extension is set to a supported one):

```python
myNode = getNode("LinearTransform_3")

myStorageNode = myNode.CreateDefaultStorageNode()
myStorageNode.SetFileName("c:/tmp/something.tfm")
myStorageNode.WriteData(myNode)
```

### Save the scene into a single MRB file

```python
# Generate file name
import time
sceneSaveFilename = slicer.app.temporaryPath + "/saved-scene-" + time.strftime("%Y%m%d-%H%M%S") + ".mrb"

# Save scene
if slicer.util.saveScene(sceneSaveFilename):
  logging.info("Scene saved to: {0}".format(sceneSaveFilename))
else:
  logging.error("Scene saving failed")
```

### Save the scene into a new directory

```python
# Create a new directory where the scene will be saved into
import time
sceneSaveDirectory = slicer.app.temporaryPath + "/saved-scene-" + time.strftime("%Y%m%d-%H%M%S")
if not os.access(sceneSaveDirectory, os.F_OK):
  os.makedirs(sceneSaveDirectory)

# Save the scene
if slicer.app.applicationLogic().SaveSceneToSlicerDataBundleDirectory(sceneSaveDirectory, None):
  logging.info("Scene saved to: {0}".format(sceneSaveDirectory))
else:
  logging.error("Scene saving failed")
```

### Override default scene save dialog

Place this class in the scripted module file to override

```python
class MyModuleFileDialog ():
  """This specially named class is detected by the scripted loadable
  module and is the target for optional drag and drop operations.
  See: Base/QTGUI/qSlicerScriptedFileDialog.h.

  This class is used for overriding default scene save dialog
  with simple saving the scene without asking anything.
  """

  def __init__(self,qSlicerFileDialog ):
    self.qSlicerFileDialog = qSlicerFileDialog
    qSlicerFileDialog.fileType = "NoFile"
    qSlicerFileDialog.description = "Save scene"
    qSlicerFileDialog.action = slicer.qSlicerFileDialog.Write

  def execDialog(self):
    # Implement custom scene save operation here.
    # Return True if saving completed successfully,
    # return False if saving was cancelled.
    ...
    return saved
```

### Override application close behavior

When application close is requested then by default confirmation popup is displayed. To customize this behavior (for example, allow application closing without displaying default confirmation popup) an event filter can be installed for the close event on the main window:

```python
class CloseApplicationEventFilter(qt.QWidget):
  def eventFilter(self, object, event):
    if event.type() == qt.QEvent.Close:
      event.accept()
      return True
    return False

filter = CloseApplicationEventFilter()
slicer.util.mainWindow().installEventFilter(filter)
```

### Change default output file type for new nodes

This script changes default output file format for nodes that have not been saved yet (do not have storage node yet).

Default node can be specified that will be used as a basis of all new storage nodes. This can be used for setting default file extension. For example, change file format to PLY for model nodes:

```python
defaultModelStorageNode = slicer.vtkMRMLModelStorageNode()
defaultModelStorageNode.SetDefaultWriteFileExtension("ply")
slicer.mrmlScene.AddDefaultNode(defaultModelStorageNode)
```

To permanently change default file extension on your computer, copy-paste the code above into your application startup script (you can find its location in menu: Edit / Application settings / General / Application startup script).

### Change file type for saving for existing nodes

This script changes output file types for nodes that have been already saved (they already have storage node).

If it is not necessary to preserve file paths then the simplest is to configure default storage node (as shown in the example above), then delete all existing storage nodes. When save dialog is opened, default storage nodes will be recreated.

```python
# Delete existing model storage nodes so that they will be recreated with default settings
existingModelStorageNodes = slicer.util.getNodesByClass("vtkMRMLModelStorageNode")
for modelStorageNode in existingModelStorageNodes:
  slicer.mrmlScene.RemoveNode(modelStorageNode)
```

To update existing storage nodes to use new file extension (but keep all other parameters unchanged) you can use this approach (example is for volume storage):

```python
requiredFileExtension = ".nia"
originalFileExtension = ".nrrd"
volumeNodes = slicer.util.getNodesByClass("vtkMRMLScalarVolumeNode")
for volumeNode in volumeNodes:
  volumeStorageNode = volumeNode.GetStorageNode()
  if not volumeStorageNode:
    volumeNode.AddDefaultStorageNode()
    volumeStorageNode = volumeNode.GetStorageNode()
    volumeStorageNode.SetFileName(volumeNode.GetName()+requiredFileExtension)
  else:
    volumeStorageNode.SetFileName(volumeStorageNode.GetFileName().replace(originalFileExtension, requiredFileExtension))
```

To set all volume nodes to save uncompressed by default (add this to [.slicerrc.py file ](../user_guide/settings.md#application-startup-file) so it takes effect for the whole session):

```python
#set the default volume storage to not compress by default
defaultVolumeStorageNode = slicer.vtkMRMLVolumeArchetypeStorageNode()
defaultVolumeStorageNode.SetUseCompression(0)
slicer.mrmlScene.AddDefaultNode(defaultVolumeStorageNode)
logging.info("Volume nodes will be stored uncompressed by default")
```

Same thing as above, but applied to all segmentations instead of volumes:

```python
#set the default volume storage to not compress by default
defaultVolumeStorageNode = slicer.vtkMRMLSegmentationStorageNode()
defaultVolumeStorageNode.SetUseCompression(0)
slicer.mrmlScene.AddDefaultNode(defaultVolumeStorageNode)
logging.info("Segmentation nodes will be stored uncompressed
```

#### Module selection

### Switch to a different module

This utility function can be used to open a different module:

```python
slicer.util.selectModule("DICOM")
```

### Set a new default module at startup

Instead of the default Welcome module:

```python
qt.QSettings().setValue("Modules/HomeModule", "Data")
```

#### Views

### Display text in a 3D view or slice view

The easiest way to show information overlaid on a viewer is to use corner annotations.

```python
view=slicer.app.layoutManager().threeDWidget(0).threeDView()
# Set text to "Something"
view.cornerAnnotation().SetText(vtk.vtkCornerAnnotation.UpperRight,"Something")
# Set color to red
view.cornerAnnotation().GetTextProperty().SetColor(1,0,0)
# Update the view
view.forceRender()
```

To display text in slice views, replace the first line by this line (and consider hiding slice view annotations, to prevent them from overwriting the text you place there):

```python
view=slicer.app.layoutManager().sliceWidget("Red").sliceView()
```

### Show orientation marker in all views

```python
viewNodes = slicer.util.getNodesByClass("vtkMRMLAbstractViewNode")
for viewNode in viewNodes:
  viewNode.SetOrientationMarkerType(slicer.vtkMRMLAbstractViewNode.OrientationMarkerTypeAxes)
```

### Change view axis labels

```python
labels = ["x", "X", "y", "Y", "z", "Z"]
viewNode = slicer.app.layoutManager().threeDWidget(0).mrmlViewNode()
# for slice view:
# viewNode = slicer.app.layoutManager().sliceWidget("Red").mrmlSliceNode()
for index, label in enumerate(labels):
  viewNode.SetAxisLabel(index, label)
```

### Hide view controller bars

```python
slicer.app.layoutManager().threeDWidget(0).threeDController().setVisible(False)
slicer.app.layoutManager().sliceWidget("Red").sliceController().setVisible(False)
slicer.app.layoutManager().plotWidget(0).plotController().setVisible(False)
slicer.app.layoutManager().tableWidget(0).tableController().setVisible(False)
```

### Hide Slicer logo from main window

This script increases vertical space available in the module panel by hiding the Slicer application logo.

```python
slicer.util.findChild(slicer.util.mainWindow(), "LogoLabel").visible = False
```

### Customize widgets in view controller bars

```python
sliceController = slicer.app.layoutManager().sliceWidget("Red").sliceController()

# hide what is not needed
sliceController.pinButton().hide()
#sliceController.viewLabel().hide()
sliceController.fitToWindowToolButton().hide()
sliceController.sliceOffsetSlider().hide()

# add custom widgets
myButton = qt.QPushButton("My custom button")
sliceController.barLayout().addWidget(myButton)
```

### Get current mouse coordinates in a slice view

You can get 3D (RAS) coordinates of the current mouse cursor from the crosshair singleton node as shown in the example below:

```python
def onMouseMoved(observer,eventid):
  ras=[0,0,0]
  crosshairNode.GetCursorPositionRAS(ras)
  print(ras)

crosshairNode=slicer.util.getNode("Crosshair")
crosshairNode.AddObserver(slicer.vtkMRMLCrosshairNode.CursorPositionModifiedEvent, onMouseMoved)
```

### Display crosshair at a 3D position

```python
position_RAS = [23.4, 5.6, 78.9]
crosshairNode = slicer.util.getNode("Crosshair")
# Set crosshair position
crosshairNode.SetCrosshairRAS(position_RAS)
# Center the position in all slice views
slicer.vtkMRMLSliceNode.JumpAllSlices(slicer.mrmlScene, *position_RAS, slicer.vtkMRMLSliceNode.CenteredJumpSlice)
# Make the crosshair visible
crosshairNode.SetCrosshairMode(slicer.vtkMRMLCrosshairNode.ShowBasic)
```

:::{note}

Crosshair node stores two positions: Cursor position is the current position of the mouse pointer in a slice or 3D view (modules should only read this position). Crosshair position is the location of the visible crosshair in views (modules can read or write this position).

:::

### Display mouse pointer coordinates in alternative coordinate system

The Data probe only shows coordinate values in the world coordinate system. You can make the world coordinate system mean anything you want (e.g., MNI) by applying a transform to the volume that transforms it into that space. See more details in [here ](https://discourse.slicer.org/t/setting-an-mni-origo-to-a-volume/16164/4).

```python
def onMouseMoved(observer,eventid):
  mniToWorldTransformNode = getNode("LinearTransform_3")  # replace this by the name of your actual MNI to world transform
  worldToMniTransform = vtk.vtkGeneralTransform()
  mniToWorldTransformNode.GetTransformToWorld(worldToMniTransform)
  ras=[0,0,0]
  mni=[0,0,0]
  crosshairNode.GetCursorPositionRAS(ras)
  worldToMniTransform.TransformPoint(ras, mni)
  _ras = "; ".join([str(k) for k in ras])
  _mni = "; ".join([str(k) for k in mni])
  slicer.util.showStatusMessage(f"RAS={_ras}   MNI={_mni}")

crosshairNode=slicer.util.getNode("Crosshair")
observationId = crosshairNode.AddObserver(slicer.vtkMRMLCrosshairNode.CursorPositionModifiedEvent, onMouseMoved)

# Run this to stop displaying values:
# crosshairNode.RemoveObserver(observationId)
```

### Get DataProbe text

You can get the mouse location in pixel coordinates along with the pixel value at the mouse by hitting the `.` (period) key in a slice view after pasting in the following code.

```python
def printDataProbe():
  infoWidget = slicer.modules.DataProbeInstance.infoWidget
  for layer in ("B", "F", "L"):
    print(infoWidget.layerNames[layer].text, infoWidget.layerIJKs[layer].text, infoWidget.layerValues[layer].text)

s = qt.QShortcut(qt.QKeySequence("."), mainWindow())
s.connect("activated()", printDataProbe)
```

### Create custom color table

This example shows how to create a new color table, for example with inverted color range from the default Ocean color table.

```python
invertedocean = slicer.vtkMRMLColorTableNode()
invertedocean.SetTypeToUser()
invertedocean.SetNumberOfColors(256)
invertedocean.SetName("InvertedOcean")

for i in range(0,255):
  invertedocean.SetColor(i, 0.0, 1 - (i+1e-16)/255.0, 1.0, 1.0)

slicer.mrmlScene.AddNode(invertedocean)
```

### Show color legend for a volume node

Display color legend for a volume node in slice views:

```python
volumeNode = getNode('MRHead')
colorLegendDisplayNode = slicer.modules.colors.logic().AddDefaultColorLegendDisplayNode(volumeNode)
```

### Create custom color map and display color legend

```python
modelNode = getNode('MyModel')  # color legend requires a displayable node
colorTableRangeMm = 40
title ="Radial\nCompression\n"
labelFormat = "%4.1f mm"

# Create color node
colorNode = slicer.mrmlScene.CreateNodeByClass("vtkMRMLProceduralColorNode")
colorNode.UnRegister(None)  # to prevent memory leaks
colorNode.SetName(slicer.mrmlScene.GenerateUniqueName("MyColormap"))
colorNode.SetAttribute("Category", "MyModule")
# The color node is a procedural color node, which is saved using a storage node.
# Hidden nodes are not saved if they use a storage node, therefore
# the color node must be visible.
colorNode.SetHideFromEditors(False)
slicer.mrmlScene.AddNode(colorNode)

# Specify colormap
colorMap = colorNode.GetColorTransferFunction()
colorMap.RemoveAllPoints()
colorMap.AddRGBPoint(colorTableRangeMm * 0.0, 0.0, 0.0, 1.0)
colorMap.AddRGBPoint(colorTableRangeMm * 0.2, 0.0, 1.0, 1.0)
colorMap.AddRGBPoint(colorTableRangeMm * 0.5, 1.0, 1.0, 0.0)
colorMap.AddRGBPoint(colorTableRangeMm * 1.0, 1.0, 0.0, 0.0)

# Display color legend
modelNode.GetDisplayNode().SetAndObserveColorNodeID(colorNode.GetID())
colorLegendDisplayNode = slicer.modules.colors.logic().AddDefaultColorLegendDisplayNode(modelNode)
colorLegendDisplayNode.SetTitleText(title)
colorLegendDisplayNode.SetLabelFormat(labelFormat)
```

### Customize view layout

Show a custom layout of a 3D view on top of the red slice view:

```python
customLayout = """
<layout type="vertical" split="true">
  <item>
  <view class="vtkMRMLViewNode" singletontag="1">
    <property name="viewlabel" action="default">1</property>
  </view>
  </item>
  <item>
  <view class="vtkMRMLSliceNode" singletontag="Red">
    <property name="orientation" action="default">Axial</property>
    <property name="viewlabel" action="default">R</property>
    <property name="viewcolor" action="default">#F34A33</property>
  </view>
  </item>
</layout>
"""

# Built-in layout IDs are all below 100, so you can choose any large random number
# for your custom layout ID.
customLayoutId=501

layoutManager = slicer.app.layoutManager()
layoutManager.layoutLogic().GetLayoutNode().AddLayoutDescription(customLayoutId, customLayout)

# Switch to the new custom layout
layoutManager.setLayout(customLayoutId)
```

See description of standard layouts (that can be used as examples) here: https://github.com/Slicer/Slicer/blob/master/Libs/MRML/Logic/vtkMRMLLayoutLogic.cxx

You can use this code snippet to add a button to the layout selector toolbar:

```python
# Add button to layout selector toolbar for this custom layout
viewToolBar = mainWindow().findChild("QToolBar", "ViewToolBar")
layoutMenu = viewToolBar.widgetForAction(viewToolBar.actions()[0]).menu()
layoutSwitchActionParent = layoutMenu  # use `layoutMenu` to add inside layout list, use `viewToolBar` to add next the standard layout list
layoutSwitchAction = layoutSwitchActionParent.addAction("My view") # add inside layout list
layoutSwitchAction.setData(customLayoutId)
layoutSwitchAction.setIcon(qt.QIcon(":Icons/Go.png"))
layoutSwitchAction.setToolTip("3D and slice view")
```

### Turn on slice intersections

```python
viewNodes = slicer.util.getNodesByClass("vtkMRMLSliceCompositeNode")
for viewNode in viewNodes:
  viewNode.SetSliceIntersectionVisibility(1)
```

:::{note}

How to find code corresponding to a user interface widget?

For this one I searched for "slice intersections" text in the whole Slicer source code, found that the function is implemented in `Base\QTGUI\qSlicerViewersToolBar.cxx`, then translated the `qSlicerViewersToolBarPrivate::setSliceIntersectionVisible(bool visible)` method to Python.

:::

### Hide slice view annotations

This script can hide node name, patient information displayed in corners of slice views (managed by DataProbe module).

```python
# Disable slice annotations immediately
sliceAnnotations = slicer.modules.DataProbeInstance.infoWidget.sliceAnnotations
sliceAnnotations.sliceViewAnnotationsEnabled=False
sliceAnnotations.updateSliceViewFromGUI()
# Disable slice annotations persistently (after Slicer restarts)
settings = qt.QSettings()
settings.setValue("DataProbe/sliceViewAnnotations.enabled", 0)
```

### Change slice offset

Equivalent to moving the slider in slice view controller.

```python
layoutManager = slicer.app.layoutManager()
red = layoutManager.sliceWidget("Red")
redLogic = red.sliceLogic()
# Print current slice offset position
print(redLogic.GetSliceOffset())
# Change slice position
redLogic.SetSliceOffset(20)
```

### Change slice orientation

Get `Red` slice node and rotate around `X` and `Y` axes.

```python
sliceNode = slicer.app.layoutManager().sliceWidget("Red").mrmlSliceNode()
sliceToRas = sliceNode.GetSliceToRAS()
transform=vtk.vtkTransform()
transform.SetMatrix(sliceToRas)
transform.RotateX(20)
transform.RotateY(15)
sliceToRas.DeepCopy(transform.GetMatrix())
sliceNode.UpdateMatrices()
```

### Measure angle between two slice planes

Measure angle between red and yellow slice nodes. Whenever any of the slice nodes are moved, the updated angle is printed on the console.

```python
sliceNodeIds = ["vtkMRMLSliceNodeRed", "vtkMRMLSliceNodeYellow"]

# Print angles between slice nodes
def ShowAngle(unused1=None, unused2=None):
  sliceNormalVector = []
  for sliceNodeId in sliceNodeIds:
    sliceToRAS = slicer.mrmlScene.GetNodeByID(sliceNodeId).GetSliceToRAS()
    sliceNormalVector.append([sliceToRAS.GetElement(0,2), sliceToRAS.GetElement(1,2), sliceToRAS.GetElement(2,2)])
  angleRad = vtk.vtkMath.AngleBetweenVectors(sliceNormalVector[0], sliceNormalVector[1])
  angleDeg = vtk.vtkMath.DegreesFromRadians(angleRad)
  print("Angle between slice planes = {0:0.3f}".format(angleDeg))

# Observe slice node changes
for sliceNodeId in sliceNodeIds:
  slicer.mrmlScene.GetNodeByID(sliceNodeId).AddObserver(vtk.vtkCommand.ModifiedEvent, ShowAngle)

# Print current angle
ShowAngle()
```

### Set slice position and orientation from a normal vector and position

This code snippet shows how to display a slice view defined by a normal vector and position in an anatomically sensible way: rotating slice view so that "up" direction (or "right" direction) is towards an anatomical axis.

```python
def setSlicePoseFromSliceNormalAndPosition(sliceNode, sliceNormal, slicePosition, defaultViewUpDirection=None, backupViewRightDirection=None):
  """
  Set slice pose from the provided plane normal and position. View up direction is determined automatically,
  to make view up point towards defaultViewUpDirection.
  :param defaultViewUpDirection Slice view will be spinned in-plane to match point approximately this up direction. Default: patient superior.
  :param backupViewRightDirection Slice view will be spinned in-plane to match point approximately this right direction
    if defaultViewUpDirection is too similar to sliceNormal. Default: patient left.
  """
  # Fix up input directions
  if defaultViewUpDirection is None:
    defaultViewUpDirection = [0,0,1]
  if backupViewRightDirection is None:
    backupViewRightDirection = [-1,0,0]
  if sliceNormal[1]>=0:
    sliceNormalStandardized = sliceNormal
  else:
    sliceNormalStandardized = [-sliceNormal[0], -sliceNormal[1], -sliceNormal[2]]
  # Compute slice axes
  sliceNormalViewUpAngle = vtk.vtkMath.AngleBetweenVectors(sliceNormalStandardized, defaultViewUpDirection)
  angleTooSmallThresholdRad = 0.25 # about 15 degrees
  if sliceNormalViewUpAngle > angleTooSmallThresholdRad and sliceNormalViewUpAngle < vtk.vtkMath.Pi() - angleTooSmallThresholdRad:
    viewUpDirection = defaultViewUpDirection
    sliceAxisY = viewUpDirection
    sliceAxisX = [0, 0, 0]
    vtk.vtkMath.Cross(sliceAxisY, sliceNormalStandardized, sliceAxisX)
  else:
    sliceAxisX = backupViewRightDirection
  # Set slice axes
  sliceNode.SetSliceToRASByNTP(sliceNormalStandardized[0], sliceNormalStandardized[1], sliceNormalStandardized[2],
    sliceAxisX[0], sliceAxisX[1], sliceAxisX[2],
    slicePosition[0], slicePosition[1], slicePosition[2], 0)

# Example usage:
sliceNode = getNode("vtkMRMLSliceNodeRed")
transformNode = getNode("Transform_3")
transformMatrix = vtk.vtkMatrix4x4()
transformNode.GetMatrixTransformToParent(transformMatrix)
sliceNormal = [transformMatrix.GetElement(0,2), transformMatrix.GetElement(1,2), transformMatrix.GetElement(2,2)]
slicePosition = [transformMatrix.GetElement(0,3), transformMatrix.GetElement(1,3), transformMatrix.GetElement(2,3)]
setSlicePoseFromSliceNormalAndPosition(sliceNode, sliceNormal, slicePosition)
```

### Show slice views in 3D window

Equivalent to clicking 'eye' icon in the slice view controller.

```python
layoutManager = slicer.app.layoutManager()
for sliceViewName in layoutManager.sliceViewNames():
  controller = layoutManager.sliceWidget(sliceViewName).sliceController()
  controller.setSliceVisible(True)
```

### Change default slice view orientation

You can left-right "flip" slice view orientation presets (show patient left side on left/right side of the screen) by copy-pasting the script below to your [.slicerrc.py file](../user_guide/settings.md#application-startup-file).

```python
# Axial slice axes:
#  1 0 0
#  0 1 0
#  0 0 1
axialSliceToRas=vtk.vtkMatrix3x3()

# Coronal slice axes:
#  1 0 0
#  0 0 -1
#  0 1 0
coronalSliceToRas=vtk.vtkMatrix3x3()
coronalSliceToRas.SetElement(1,1, 0)
coronalSliceToRas.SetElement(1,2, -1)
coronalSliceToRas.SetElement(2,1, 1)
coronalSliceToRas.SetElement(2,2, 0)

# Replace orientation presets in all existing slice nodes and in the default slice node
sliceNodes = slicer.util.getNodesByClass("vtkMRMLSliceNode")
sliceNodes.append(slicer.mrmlScene.GetDefaultNodeByClass("vtkMRMLSliceNode"))
for sliceNode in sliceNodes:
  orientationPresetName = sliceNode.GetOrientation()
  sliceNode.RemoveSliceOrientationPreset("Axial")
  sliceNode.AddSliceOrientationPreset("Axial", axialSliceToRas)
  sliceNode.RemoveSliceOrientationPreset("Coronal")
  sliceNode.AddSliceOrientationPreset("Coronal", coronalSliceToRas)
  sliceNode.SetOrientation(orientationPresetName)
```

### Set all slice views linked by default

You can make slice views linked by default (when application starts or the scene is cleared) by copy-pasting the script below to your [.slicerrc.py file ](../user_guide/settings.md#application-startup-file).

```python
# Set linked slice views  in all existing slice composite nodes and in the default node
sliceCompositeNodes = slicer.util.getNodesByClass("vtkMRMLSliceCompositeNode")
defaultSliceCompositeNode = slicer.mrmlScene.GetDefaultNodeByClass("vtkMRMLSliceCompositeNode")
if not defaultSliceCompositeNode:
  defaultSliceCompositeNode = slicer.mrmlScene.CreateNodeByClass("vtkMRMLSliceCompositeNode")
  defaultSliceCompositeNode.UnRegister(None)  # CreateNodeByClass is factory method, need to unregister the result to prevent memory leaks
  slicer.mrmlScene.AddDefaultNode(defaultSliceCompositeNode)
sliceCompositeNodes.append(defaultSliceCompositeNode)
for sliceCompositeNode in sliceCompositeNodes:
  sliceCompositeNode.SetLinkedControl(True)
```

### Set crosshair jump mode to centered by default

You can change default slice jump mode (when application starts or the scene is cleared) by copy-pasting the script below to your [.slicerrc.py file ](../user_guide/settings.md#application-startup-file).

```python
crosshair=slicer.mrmlScene.GetFirstNodeByClass("vtkMRMLCrosshairNode")
crosshair.SetCrosshairBehavior(crosshair.CenteredJumpSlice)
```

### Set up custom units in slice view ruler

For microscopy or micro-CT images you may want to switch unit to micrometer instead of the default mm. To do that, 1. change the unit in Application settings / Units and 2. update ruler display settings using the script below (it can be copied to your Application startup script):

```python
lm = slicer.app.layoutManager()
for sliceViewName in lm.sliceViewNames():
  sliceView = lm.sliceWidget(sliceViewName).sliceView()
  displayableManager = sliceView.displayableManagerByClassName("vtkMRMLRulerDisplayableManager")
  displayableManager.RemoveAllRulerScalePresets()
  displayableManager.AddRulerScalePreset(   0.001, 5, 2, "nm", 1000.0)
  displayableManager.AddRulerScalePreset(   0.010, 5, 2, "nm", 1000.0)
  displayableManager.AddRulerScalePreset(   0.100, 5, 2, "nm", 1000.0)
  displayableManager.AddRulerScalePreset(   0.500, 5, 1, "nm", 1000.0)
  displayableManager.AddRulerScalePreset(   1.0,   5, 2, "um",    1.0)
  displayableManager.AddRulerScalePreset(   5.0,   5, 1, "um",    1.0)
  displayableManager.AddRulerScalePreset(  10.0,   5, 2, "um",    1.0)
  displayableManager.AddRulerScalePreset(  50.0,   5, 1, "um",    1.0)
  displayableManager.AddRulerScalePreset( 100.0,   5, 2, "um",    1.0)
  displayableManager.AddRulerScalePreset( 500.0,   5, 1, "um",    1.0)
  displayableManager.AddRulerScalePreset(1000.0,   5, 2, "mm",    0.001)
```

### Center the 3D view on the scene

```python
layoutManager = slicer.app.layoutManager()
threeDWidget = layoutManager.threeDWidget(0)
threeDView = threeDWidget.threeDView()
threeDView.resetFocalPoint()
```

### Rotate the 3D View

```python
layoutManager = slicer.app.layoutManager()
threeDWidget = layoutManager.threeDWidget(0)
threeDView = threeDWidget.threeDView()
threeDView.yaw()
```

### Change 3D view background color

```python
viewNode = slicer.app.layoutManager().threeDWidget(0).mrmlViewNode()
viewNode.SetBackgroundColor(1,0,0)
viewNode.SetBackgroundColor2(1,0,0)
```

### Show a slice view outside the view layout

```python
# layout name is used to create and identify the underlying slice node and  should be set to a value that is not used in any of the layouts owned by the layout manager
layoutName = "TestSlice1"
layoutLabel = "TS1"
layoutColor = [1.0, 1.0, 0.0]
# ownerNode manages this view instead of the layout manager (it can be any node in the scene)
viewOwnerNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode")

# Create MRML nodes
viewLogic = slicer.vtkMRMLSliceLogic()
viewLogic.SetMRMLScene(slicer.mrmlScene)
viewNode = viewLogic.AddSliceNode(layoutName)
viewNode.SetLayoutLabel(layoutLabel)
viewNode.SetLayoutColor(layoutColor)
viewNode.SetAndObserveParentLayoutNodeID(viewOwnerNode.GetID())

# Create widget
viewWidget = slicer.qMRMLSliceWidget()
viewWidget.setMRMLScene(slicer.mrmlScene)
viewWidget.setMRMLSliceNode(viewNode)
sliceLogics = slicer.app.applicationLogic().GetSliceLogics()
viewWidget.setSliceLogics(sliceLogics)
sliceLogics.AddItem(viewWidget.sliceLogic())
viewWidget.show()
```

### Show a 3D view outside the view layout

```python
# layout name is used to create and identify the underlying view node and  should be set to a value that is not used in any of the layouts owned by the layout manager
layoutName = "Test3DView"
layoutLabel = "T3"
layoutColor = [1.0, 1.0, 0.0]
# ownerNode manages this view instead of the layout manager (it can be any node in the scene)
viewOwnerNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode")

# Create MRML node
viewLogic = slicer.vtkMRMLViewLogic()
viewLogic.SetMRMLScene(slicer.mrmlScene)
viewNode = viewLogic.AddViewNode(layoutName)
viewNode.SetLayoutLabel(layoutLabel)
viewNode.SetLayoutColor(layoutColor)
viewNode.SetAndObserveParentLayoutNodeID(viewOwnerNode.GetID())

# Create widget
viewWidget = slicer.qMRMLThreeDWidget()
viewWidget.setMRMLScene(slicer.mrmlScene)
viewWidget.setMRMLViewNode(viewNode)
viewWidget.show()
```

#### Access VTK rendering classes

### Access VTK views, renderers, and cameras

Iterate through all 3D views in current layout:

```python
layoutManager = slicer.app.layoutManager()
for threeDViewIndex in range(layoutManager.threeDViewCount) :
  view = layoutManager.threeDWidget(threeDViewIndex).threeDView()
  threeDViewNode = view.mrmlViewNode()
  cameraNode = slicer.modules.cameras.logic().GetViewActiveCameraNode(threeDViewNode)
  print("View node for 3D widget " + str(threeDViewIndex))
  print("  Name: " + threeDViewNode .GetName())
  print("  ID: " + threeDViewNode .GetID())
  print("  Camera ID: " + cameraNode.GetID())
```

Iterate through all slice views in current layout:

```python
layoutManager = slicer.app.layoutManager()
for sliceViewName in layoutManager.sliceViewNames():
  view = layoutManager.sliceWidget(sliceViewName).sliceView()
  sliceNode = view.mrmlSliceNode()
  sliceLogic = slicer.app.applicationLogic().GetSliceLogic(sliceNode)
  compositeNode = sliceLogic.GetSliceCompositeNode()
  print("Slice view " + str(sliceViewName))
  print("  Name: " + sliceNode.GetName())
  print("  ID: " + sliceNode.GetID())
  print("  Background volume: {0}".format(compositeNode.GetBackgroundVolumeID()))
  print("  Foreground volume: {0} (opacity: {1})".format(compositeNode.GetForegroundVolumeID(), compositeNode.GetForegroundOpacity()))
  print("  Label volume: {0} (opacity: {1})".format(compositeNode.GetLabelVolumeID(), compositeNode.GetLabelOpacity()))
```

For low-level manipulation of views, it is possible to access VTK render windows, renderers and cameras of views in the current layout.

```python
renderWindow = view.renderWindow()
renderers = renderWindow.GetRenderers()
renderer = renderers.GetItemAsObject(0)
camera = cameraNode.GetCamera()
```

### Get displayable manager of a certain type for a certain view

Displayable managers are responsible for creating VTK filters, mappers, and actors to display MRML nodes in renderers. Input to filters and mappers are VTK objects stored in MRML data nodes. Filter and actor properties are set based on display options specified in MRML display nodes.

Accessing displayable managers is useful for troubleshooting or for testing new features that are not exposed via MRML classes yet, as they provide usually allow low-level access to VTK actors.

```python
threeDViewWidget = slicer.app.layoutManager().threeDWidget(0)
modelDisplayableManager = threeDViewWidget.threeDView().displayableManagerByClassName("vtkMRMLModelDisplayableManager")
if modelDisplayableManager is None:
  logging.error("Failed to find the model displayable manager")
```

### Access VTK actor properties

This example shows how to access and modify VTK actor properties to experiment with physically-based rendering.

```python
modelNode = slicer.util.getNode("MyModel")

threeDViewWidget = slicer.app.layoutManager().threeDWidget(0)
modelDisplayableManager = threeDViewWidget.threeDView().displayableManagerByClassName("vtkMRMLModelDisplayableManager")
actor=modelDisplayableManager.GetActorByID(modelNode.GetDisplayNode().GetID())
property=actor.GetProperty()
property.SetInterpolationToPBR()
property.SetMetallic(0.5)
property.SetRoughness(0.5)
property.SetColor(0.5,0.5,0.9)
slicer.util.forceRenderAllViews()
```

See more information on physically based rendering in VTK here: https://blog.kitware.com/vtk-pbr/

#### Keyboard shortcuts and mouse gestures

### Customize keyboard shortcuts

Keyboard shortcuts can be specified for activating any Slicer feature by adding a couple of lines to your [.slicerrc.py file](../user_guide/settings.md#application-startup-file).

For example, this script registers <kbd>Ctrl+b</kbd>, <kbd>Ctrl+n</kbd>, <kbd>Ctrl+m</kbd>, <kbd>Ctrl+,</kbd> keyboard shortcuts to switch between red, yellow, green, and 4-up view layouts.

```python
shortcuts = [
  ("Ctrl+b", lambda: slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)),
  ("Ctrl+n", lambda: slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpYellowSliceView)),
  ("Ctrl+m", lambda: slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpGreenSliceView)),
  ("Ctrl+,", lambda: slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView))
  ]

for (shortcutKey, callback) in shortcuts:
  shortcut = qt.QShortcut(slicer.util.mainWindow())
  shortcut.setKey(qt.QKeySequence(shortcutKey))
  shortcut.connect( "activated()", callback)
```

Here's an example for cycling through Segment Editor effects (requested [on the Slicer forum](https://discourse.slicer.org/t/is-there-a-keystroke-to-cycle-through-effects-in-segment-editor/10117/2) for the [SlicerMorph](http://slicermorph.org) project).

```python
def cycleEffect(delta=1):
  try:
    orderedNames = list(slicer.modules.SegmentEditorWidget.editor.effectNameOrder())
    allNames = slicer.modules.SegmentEditorWidget.editor.availableEffectNames()
    for name in allNames:
      try:
        orderedNames.index(name)
      except ValueError:
        orderedNames.append(name)
    orderedNames.insert(0, None)
    activeEffect = slicer.modules.SegmentEditorWidget.editor.activeEffect()
    if activeEffect:
      activeName = slicer.modules.SegmentEditorWidget.editor.activeEffect().name
    else:
      activeName = None
    newIndex = (orderedNames.index(activeName) + delta) % len(orderedNames)
    slicer.modules.SegmentEditorWidget.editor.setActiveEffectByName(orderedNames[newIndex])
  except AttributeError:
    # module not active
    pass

shortcuts = [
  ("`", lambda: cycleEffect(-1)),
  ("~", lambda: cycleEffect(1)),
  ]

for (shortcutKey, callback) in shortcuts:
  shortcut = qt.QShortcut(slicer.util.mainWindow())
  shortcut.setKey(qt.QKeySequence(shortcutKey))
  shortcut.connect( "activated()", callback)
```

### Customize keyboard/mouse gestures in viewers

#### Make the 3D view rotate using right-click-and-drag

```python
threeDViewWidget = slicer.app.layoutManager().threeDWidget(0)
cameraDisplayableManager = threeDViewWidget.threeDView().displayableManagerByClassName("vtkMRMLCameraDisplayableManager")
cameraWidget = cameraDisplayableManager.GetCameraWidget()

# Remove old mapping from right-click-and-drag
cameraWidget.SetEventTranslationClickAndDrag(cameraWidget.WidgetStateIdle, vtk.vtkCommand.RightButtonPressEvent, vtk.vtkEvent.NoModifier,
  cameraWidget.WidgetStateRotate, vtk.vtkWidgetEvent.NoEvent, vtk.vtkWidgetEvent.NoEvent)

# Make right-click-and-drag rotate the view
cameraWidget.SetEventTranslationClickAndDrag(cameraWidget.WidgetStateIdle, vtk.vtkCommand.RightButtonPressEvent, vtk.vtkEvent.NoModifier,
  cameraWidget.WidgetStateRotate, cameraWidget.WidgetEventRotateStart, cameraWidget.WidgetEventRotateEnd)
```

#### Add shortcut to adjust window/level in any mouse mode

Makes Ctrl + Right-click-and-drag gesture adjust window/level in red slice view. This gesture works even when not in "Adjust window/level" mouse mode.

```python
sliceViewLabel = "Red"
sliceViewWidget = slicer.app.layoutManager().sliceWidget(sliceViewLabel)
displayableManager = sliceViewWidget.sliceView().displayableManagerByClassName("vtkMRMLScalarBarDisplayableManager")
w = displayableManager.GetWindowLevelWidget()
w.SetEventTranslationClickAndDrag(w.WidgetStateIdle,
  vtk.vtkCommand.RightButtonPressEvent, vtk.vtkEvent.ControlModifier,
  w.WidgetStateAdjustWindowLevel, w.WidgetEventAlwaysOnAdjustWindowLevelStart, w.WidgetEventAlwaysOnAdjustWindowLevelEnd)
```

### Disable certain user interactions in slice views

For example, disable slice browsing using mouse wheel and keyboard shortcuts in the red slice viewer:

```python
interactorStyle = slicer.app.layoutManager().sliceWidget("Red").sliceView().sliceViewInteractorStyle()
interactorStyle.SetActionEnabled(interactorStyle.BrowseSlice, False)
```

Hide all slice view controllers:

```python
lm = slicer.app.layoutManager()
for sliceViewName in lm.sliceViewNames():
  lm.sliceWidget(sliceViewName).sliceController().setVisible(False)
```

Hide all 3D view controllers:

```python
lm = slicer.app.layoutManager()
for viewIndex in range(slicer.app.layoutManager().threeDViewCount):
  lm.threeDWidget(0).threeDController().setVisible(False)
```

### Add keyboard shortcut to jump to center or world coordinate system

You can copy-paste this into the Python console to jump slice views to (0,0,0) position on (Ctrl+e):

```python
shortcut = qt.QShortcut(qt.QKeySequence("Ctrl+e"), slicer.util.mainWindow())
shortcut.connect("activated()",
  lambda: slicer.modules.markups.logic().JumpSlicesToLocation(0,0,0, True))
```

#### Launch external applications

How to run external applications from Slicer.

### Launch external process in startup environment

When a process is launched from Slicer then by default Slicer"s ITK, VTK, Qt, etc. libraries are used. If an external application has its own version of these libraries, then the application is expected to crash. To prevent crashing, the application must be run in the environment where Slicer started up (without all Slicer-specific library paths). This startup environment can be retrieved using {func}`slicer.util.startupEnvironment()`.

Example: run Python3 script from Slicer:

```python
command_to_execute = ["/usr/bin/python3", "-c", "print("hola")"]
from subprocess import check_output
check_output(
  command_to_execute,
  env=slicer.util.startupEnvironment()
  )
```

will output:

```python
"hola\n"
```

On some systems, *shell=True* must be specified as well.

#### Manage extensions

### Download and install extension

```python
extensionName = 'SlicerIGT'
em = slicer.app.extensionsManagerModel()
if not em.isExtensionInstalled(extensionName):
  extensionMetaData = em.retrieveExtensionMetadataByName(extensionName)
  url = f"{em.serverUrl().toString()}/api/v1/item/{extensionMetaData['_id']}/download"
  extensionPackageFilename = slicer.app.temporaryPath+'/'+extensionMetaData['_id']
  slicer.util.downloadFile(url, extensionPackageFilename)
  em.installExtension(extensionPackageFilename)
  slicer.util.restart()
```

### Install a module directly from a git repository

This [code snippet](https://gist.github.com/pieper/a9c0ba57de3833c9f5aea68247bda597) can be useful for sharing code in development without requiring a restart of Slicer.

### Install a Python package

Python packages that are optional or would be impractical to bundle into the extension can be installed at runtime. It is recommended to only install a package when it is actually needed (not at startup, not even when the user opens a module, but just before that Python package is used the first time), and ask the user about it (if it is more than just a few megabytes).

```python
try:
  import flywheel
except ModuleNotFoundError as e:
  if slicer.util.confirmOkCancelDisplay("This module requires 'flywheel-sdk' Python package. Click OK to install it now."):
    slicer.util.pip_install("flywheel-sdk")
    import flywheel
```
