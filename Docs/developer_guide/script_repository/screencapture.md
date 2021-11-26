## Screen Capture

### Capture the full Slicer screen and save it into a file

```python
img = qt.QPixmap.grabWidget(slicer.util.mainWindow()).toImage()
img.save("c:/tmp/test.png")
```

### Capture all the views save it into a file

```python
import ScreenCapture
cap = ScreenCapture.ScreenCaptureLogic()
cap.showViewControllers(False)
cap.captureImageFromView(None, "c:/tmp/test.png")
cap.showViewControllers(True)
```

### Capture a single view

```python
viewNodeID = "vtkMRMLViewNode1"
import ScreenCapture
cap = ScreenCapture.ScreenCaptureLogic()
view = cap.viewFromNode(slicer.mrmlScene.GetNodeByID(viewNodeID))
cap.captureImageFromView(view, "c:/tmp/test.png")
```

Common values for viewNodeID: vtkMRMLSliceNodeRed, vtkMRMLSliceNodeYellow, vtkMRMLSliceNodeGreen, vtkMRMLViewNode1, vtkMRMLViewNode2. The ScreenCapture module can also create video animations of rotating views, slice sweeps, etc.

### Capture a slice view sweep into a series of PNG files

For example, Red slice view, 30 images, from position -125.0 to 75.0, into c:/tmp folder, with name image_00001.png, image_00002.png, ...

```python
import ScreenCapture
ScreenCapture.ScreenCaptureLogic().captureSliceSweep(getNode("vtkMRMLSliceNodeRed"), -125.0, 75.0, 30, "c:/tmp", "image_%05d.png")
```

### Capture 3D view into PNG file with transparent background

```python
# Set background to black (required for transparent background)
view = slicer.app.layoutManager().threeDWidget(0).threeDView()
view.mrmlViewNode().SetBackgroundColor(0,0,0)
view.mrmlViewNode().SetBackgroundColor2(0,0,0)
# Capture RGBA image
renderWindow = view.renderWindow()
renderWindow.SetAlphaBitPlanes(1)
wti = vtk.vtkWindowToImageFilter()
wti.SetInputBufferTypeToRGBA()
wti.SetInput(renderWindow)
writer = vtk.vtkPNGWriter()
writer.SetFileName("c:/tmp/screenshot.png")
writer.SetInputConnection(wti.GetOutputPort())
writer.Write()
```

### Capture slice view into PNG file with white background

```python
sliceViewName = "Red"
filename = "c:/tmp/screenshot.png"

# Set view background to white
view = slicer.app.layoutManager().sliceWidget(sliceViewName).sliceView()
view.setBackgroundColor(qt.QColor.fromRgbF(1,1,1))
view.forceRender()

# Capture a screenshot
import ScreenCapture
cap = ScreenCapture.ScreenCaptureLogic()
cap.captureImageFromView(view, filename)
```

### Save a series of images from a slice view

You can use ScreenCapture module to capture series of images. To do it programmatically, save the following into a file such as ``/tmp/record.py`` and then in the Slicer python console type ``execfile("/tmp/record.py")``

```python
layoutName = "Green"
imagePathPattern = "/tmp/image-%03d.png"
steps = 10

widget = slicer.app.layoutManager().sliceWidget(layoutName)
view = widget.sliceView()
logic = widget.sliceLogic()
bounds = [0,]*6
logic.GetSliceBounds(bounds)

for step in range(steps):
  offset = bounds[4] + step/(1.*steps) * (bounds[5]-bounds[4])
  logic.SetSliceOffset(offset)
  view.forceRender()
  image = qt.QPixmap.grabWidget(view).toImage()
  image.save(imagePathPattern % step)
```
