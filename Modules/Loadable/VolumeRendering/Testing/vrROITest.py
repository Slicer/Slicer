import slicer
import qt
import os
# TODO: this won't be needed once it is done automatically at module discovery
#from libVolumeRenderingMRMLPython import *

def load_default_volume():
  if not getNodes().has_key('moving'):
    fileName = "c:/alexy/Slicer4/Libs/MRML/Core/Testing/TestData/moving.nrrd"
    #fileName = os.environ['SLICER_HOME'] + "/share/MRML/Testing/TestData/moving.nrrd"
    #fileName = os.environ['HOME'] + "/Dropbox/data/faces/neutral.nrrd"
    vl = slicer.modules.volumes.logic()
    volumeNode = vl.AddArchetypeScalarVolume (fileName, "moving", 0)
    # automatically select the volume to display
    mrmlLogic = slicer.app.applicationLogic()
    selNode = mrmlLogic.GetSelectionNode()
    selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
    mrmlLogic.PropagateVolumeSelection()


load_default_volume()
volumeNode = getNode('moving')

logic = slicer.modules.volumerendering.logic()

displayNode = logic.CreateVolumeRenderingDisplayNode()

viewNode = getNode('ViewNode')
displayNode.AddViewNodeID(viewNode.GetID())
logic.UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode)


# TODO: actually select the module GUI
#slicer.modules.volumerendering.widgetRepresentation()

def viewImageData():
  layoutManager = slicer.app.layoutManager()
  view = layoutManager.threeDWidget(0).threeDView()
  w2i = vtk.vtkWindowToImageFilter()
  w2i.SetInput(view.renderWindow())
  w2i.Update()
  v = vtk.vtkImageViewer()
  v.SetColorWindow(255)
  v.SetColorLevel(128)
  v.SetInput(w2i.GetOutput())
  v.Render()
  return v


def moveROI(x,y,z):
  print("move roi to" , x,y,z)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  roin.SetXYZ(x,y,z)
  roin.Modified()
  v = viewImageData()

def moveROIX(x):
  print("move roi x to" , x)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  xyz = [0,0,0]
  roin.GetXYZ(xyz)
  roin.SetXYZ(x,xyz[1],xyz[2])
  roin.Modified()
  v = viewImageData()

def moveROIY(y):
  print("move roi y to" , y)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  xyz = [0,0,0]
  roin.GetXYZ(xyz)
  roin.SetXYZ(xyz[0],y,xyz[2])
  roin.Modified()
  v = viewImageData()

def moveROIZ(z):
  print("move roi z to" , z)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  xyz = [0,0,0]
  roin.GetXYZ(xyz)
  roin.SetXYZ(xyz[0],xyz[1],z)
  roin.Modified()
  v = viewImageData()

def sizeROI(x,y,z):
  print("set roi rad to " , x,y,z)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  roin.SetRadiusXYZ(x,y,z)
  roin.Modified()
  v = viewImageData()

def sizeROIX(x):
  print("set roi x rad to " , x)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  xyz = [0,0,0]
  roin.GetRadiusXYZ(xyz)
  roin.SetRadiusXYZ(x,xyz[1],xyz[2])
  roin.Modified()
  v = viewImageData()

def sizeROIY(y):
  print("set roi y rad to " , y)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  xyz = [0,0,0]
  roin.GetRadiusXYZ(xyz)
  roin.SetRadiusXYZ(xyz[0],y,xyz[2])
  roin.Modified()
  v = viewImageData()

def sizeROIZ(z):
  print("set roi z rad to " , z)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vrdn.SetCroppingEnabled(1)
  roin = vrdn.GetROINode()
  xyz = [0,0,0]
  roin.GetRadiusXYZ(xyz)
  roin.SetRadiusXYZ(xyz[0],xyz[1],z)
  roin.Modified()
  v = viewImageData()

# TODO: how to get the test mode from the environment?
testMode = False
if testMode:
  import time
  moveROI(0,0,0)
  slicer.app.processEvents()
  time.sleep(5)
  moveROI(100,100,100)
  slicer.app.processEvents()
  time.sleep(5)
  moveROI(10,10,10)
  slicer.app.processEvents()
  time.sleep(5)
  moveROIX(50)
  slicer.app.processEvents()
  time.sleep(5)
  moveROIY(50)
  slicer.app.processEvents()
  time.sleep(5)
  moveROIZ(50)
  slicer.app.processEvents()
  time.sleep(5)
  sizeROI(10,20,40)
  slicer.app.processEvents()
  time.sleep(5)
  sizeROIX(50)
  slicer.app.processEvents()
  time.sleep(5)
  sizeROIY(50)
  slicer.app.processEvents()
  time.sleep(5)
  sizeROIZ(50)
  slicer.app.processEvents()
  time.sleep(5)
  exit()
else:
  control = qt.QWidget()
  layout = qt.QVBoxLayout()
  control.setLayout(layout)
  sX = qt.QSlider(qt.Qt.Horizontal,control)
  layout.addWidget(sX)
  sX.connect('valueChanged(int)',  moveROIX)
  sY = qt.QSlider(qt.Qt.Horizontal,control)
  layout.addWidget(sY)
  sY.connect('valueChanged(int)',  moveROIY)
  sZ = qt.QSlider(qt.Qt.Horizontal,control)
  layout.addWidget(sZ)
  sZ.connect('valueChanged(int)',  moveROIZ)
  sX1 = qt.QSlider(qt.Qt.Horizontal,control)
  layout.addWidget(sX1)
  sX1.connect('valueChanged(int)',  sizeROIX)
  sY1 = qt.QSlider(qt.Qt.Horizontal,control)
  layout.addWidget(sY1)
  sY1.connect('valueChanged(int)',  sizeROIY)
  sZ1 = qt.QSlider(qt.Qt.Horizontal,control)
  layout.addWidget(sZ1)
  sZ1.connect('valueChanged(int)',  sizeROIZ)
  be = qt.QPushButton("Exit")
  layout.addWidget(be)
  be.connect('clicked()', exit)
  p = mainWindow().geometry
  control.setGeometry(p.x()-30, p.y()-30, 300, 700)
  control.show()


