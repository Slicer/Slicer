from __future__ import print_function

# Test that setting a fiducial coordinate is consistent across scene view saves
# and restores

displayNode = slicer.vtkMRMLMarkupsDisplayNode()
slicer.mrmlScene.AddNode(displayNode)

fid = slicer.vtkMRMLMarkupsFiducialNode()
slicer.mrmlScene.AddNode(fid)

fid.SetAndObserveDisplayNodeID(displayNode.GetID())

startCoords = [1.0, 2.0, 3.0]
fid.AddFiducialFromArray(startCoords)

fid.GetNthFiducialPosition(0,startCoords)
print("Starting fiducial coordinates = ",startCoords)

sv = slicer.mrmlScene.AddNode(slicer.vtkMRMLSceneViewNode())

sv.StoreScene()

afterStoreSceneCoords = [11.1,22.2,33.3]
fid.SetNthFiducialPositionFromArray(0, afterStoreSceneCoords)

fid.GetNthFiducialPosition(0,afterStoreSceneCoords)
print("After storing the scene, set fiducial coords to ",afterStoreSceneCoords)

sv.RestoreScene()

fidAfterRestore =  slicer.mrmlScene.GetNodeByID("vtkMRMLMarkupsFiducialNode1")

coords = [0,0,0]
fidAfterRestore.GetNthFiducialPosition(0,coords)
print("After restoring the scene, fiducial coordinates = ", coords)

xdiff = coords[0] - startCoords[0]
ydiff = coords[1] - startCoords[1]
zdiff = coords[2] - startCoords[2]

print("Difference between coordinates after restored the scene and value from just before stored the scene: ", xdiff, ydiff, zdiff)

diffTotal = xdiff + ydiff + zdiff

if diffTotal > 0.1:
  exceptionMessage = "Difference between coordinate values total = " + str(diffTotal)
  raise Exception(exceptionMessage)
