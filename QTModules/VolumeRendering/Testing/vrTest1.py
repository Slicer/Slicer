import slicer
import qt
import os
# TODO: this won't be needed once it is done automatically at module discovery
from libVolumeRenderingMRMLPython import *

def load_default_volume():
  if not getNodes().has_key('moving'):
    #fileName = os.environ['Slicer_HOME'] + "/share/MRML/Testing/TestData/moving.nrrd"
    fileName = os.environ['HOME'] + "/Dropbox/data/faces/neutral.nrrd"
    vl = slicer.modules.volumes.logic()
    volumeNode = vl.AddArchetypeScalarVolume (fileName, "moving", 0)

    # automatically select the volume to display
    mrmlLogic = slicer.app.mrmlApplicationLogic()
    selNode = mrmlLogic.GetSelectionNode()
    selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
    mrmlLogic.PropagateVolumeSelection()


load_default_volume()
slicer.app.processEvents()

# TODO: actually select the module GUI 
slicer.modules.volumerendering.widgetRepresentation()



def vr1():
  print("set params for 1")
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vp = vpn.GetVolumeProperty()

  vrdn.SetUseThreshold(1)
  vrdn.SetThreshold(3000,5000)

def vr2():
  print("set params for 2")
  vrdn = getNode('vtkMRMLVolumeRenderingDisplayNode')
  vpn = vrdn.GetVolumePropertyNode()
  vp = vpn.GetVolumeProperty()

  vrdn.SetUseThreshold(1)
  vrdn.SetThreshold(300,5000)


control = qt.QWidget()
layout = qt.QVBoxLayout()
control.setLayout(layout)

b1 = qt.QPushButton("Step 1")
layout.addWidget(b1)
b1.connect('clicked()', vr1)

b2 = qt.QPushButton("Step 2")
layout.addWidget(b2)
b2.connect('clicked()', vr2)

b2 = qt.QPushButton("Exit")
layout.addWidget(b2)
b2.connect('clicked()', exit)

p = mainWindow().geometry
control.setGeometry(p.x()-30, p.y()-30, 300, 700)
control.show()

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


