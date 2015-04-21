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



def vr0():
  print("0: VTK CPU ray cast")
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vrdn.SetCurrentVolumeMapper(0)
  global v
  v = viewImageData()


def vr1():
  print("1: VTK GPU ray cast")
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vrdn.SetCurrentVolumeMapper(1)

def vr2():
  print("2: VTK GPU texture mapping")
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vrdn.SetCurrentVolumeMapper(2)

def vr3():
  print("3: NCI GPU ray cast")
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vrdn.SetCurrentVolumeMapper(3)

def vr4():
  print("4: NCI GPU ray cast (multi-volume)")
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vrdn.SetCurrentVolumeMapper(4)

def vrthres(value):
  print("set theshold to", value)
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vp = vpn.GetVolumeProperty()
  op = vp.GetScalarOpacity()
  op.RemoveAllPoints()
  op.AddPoint(value,0)
  op.AddPoint(value+0.01,1)
  vp.Modified()



# TODO: how to get the test mode from the environment?
testMode = False
if testMode:
  import time
  vr1()
  slicer.app.processEvents()
  time.sleep(5)
  vr2()
  slicer.app.processEvents()
  time.sleep(5)
  exit()
else:
  control = qt.QWidget()
  layout = qt.QVBoxLayout()
  control.setLayout(layout)
  b0 = qt.QPushButton("0: VTK CPU ray cast")
  layout.addWidget(b0)
  b0.connect('clicked()', vr0)
  b1 = qt.QPushButton("1: VTK GPU ray cast")
  layout.addWidget(b1)
  b1.connect('clicked()', vr1)
  b2 = qt.QPushButton("2: VTK GPU texture mapping")
  layout.addWidget(b2)
  b2.connect('clicked()', vr2)
  b3 = qt.QPushButton("3: NCI GPU ray cast")
  layout.addWidget(b3)
  b3.connect('clicked()', vr3)
  b4 = qt.QPushButton("4: NCI GPU ray cast (multi-volume)")
  layout.addWidget(b4)
  b4.connect('clicked()', vr4)
  s1 = qt.QSlider(qt.Qt.Horizontal,control)
  layout.addWidget(s1)
  s1.connect('valueChanged(int)', vrthres)
  be = qt.QPushButton("Exit")
  layout.addWidget(be)
  be.connect('clicked()', exit)
  p = mainWindow().geometry
  control.setGeometry(p.x()-30, p.y()-30, 300, 700)
  control.show()


