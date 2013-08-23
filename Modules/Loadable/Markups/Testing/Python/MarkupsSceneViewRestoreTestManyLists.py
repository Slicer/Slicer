
# Test restoring a scene with multiple lists with different number
# of fiducials

# first fiducial list
displayNode1 = slicer.vtkMRMLMarkupsDisplayNode()
slicer.mrmlScene.AddNode(displayNode1)

fidNode1 = slicer.vtkMRMLMarkupsFiducialNode()
fidNode1.SetName("FidNode1")
slicer.mrmlScene.AddNode(fidNode1)

fidNode1.SetAndObserveDisplayNodeID(displayNode1.GetID())


coords = [0.0, 0.0, 0.0]
numFidsInList1 = 5
for i in range(numFidsInList1):
  fidNode1.AddFiducialFromArray(coords)
  coords[0] += 1.0
  coords[1] += 2.0
  coords[2] += 1.0

# second fiducial list
displayNode2 = slicer.vtkMRMLMarkupsDisplayNode()
slicer.mrmlScene.AddNode(displayNode2)

fidNode2 = slicer.vtkMRMLMarkupsFiducialNode()
fidNode2.SetName("FidNode2")
slicer.mrmlScene.AddNode(fidNode2)

fidNode2.SetAndObserveDisplayNodeID(displayNode2.GetID())

numFidsInList2 = 10
for i in range(numFidsInList2):
  fidNode2.AddFiducialFromArray(coords)
  coords[0] += 1.0
  coords[1] += 1.0
  coords[2] += 3.0



sv = slicer.mrmlScene.AddNode(slicer.vtkMRMLSceneViewNode())

numFidNodesBeforeStore = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialNode')

sv.StoreScene()

# add a third list that will get removed on restore
# second fiducial list
displayNode3 = slicer.vtkMRMLMarkupsDisplayNode()
slicer.mrmlScene.AddNode(displayNode3)

fidNode3 = slicer.vtkMRMLMarkupsFiducialNode()
fidNode3.SetName("FidNode3")
slicer.mrmlScene.AddNode(fidNode3)

fidNode3.SetAndObserveDisplayNodeID(displayNode3.GetID())

numFidsInList3 = 2
for i in range(numFidsInList3):
  fidNode3.AddFiducialFromArray(coords)
  coords[0] += 1.0
  coords[1] += 2.0
  coords[2] += 3.0


sv.RestoreScene()

numFidNodesAfterRestore = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialNode')

if numFidNodesAfterRestore != numFidNodesBeforeStore:
  print "After restoring the scene, expected ", numFidNodesBeforeStore, " fiducial nodes, but have ", numFidNodesAfterRestore
  exceptionMessage = "After restoring the scene, expected " + str(numFidNodesBeforeStore) + " fiducial nodes, but have " + str(numFidNodesAfterRestore)
  raise Exception(exceptionMessage)

#fid1AfterRestore = slicer.mrmlScene.GetNodeByID("vtkMRMLMarkupsFiducialNode1")
fid1AfterRestore = slicer.mrmlScene.GetFirstNodeByName("FidNode1")
numFidsInList1AfterRestore = fid1AfterRestore.GetNumberOfMarkups()
print "After restore, list with name FidNode1 has id ", fid1AfterRestore.GetID(), " and num fids = ", numFidsInList1AfterRestore
if numFidsInList1AfterRestore != numFidsInList1:
  exceptionMessage = "After restoring list 1, id = " + fid1AfterRestore.GetID()
  exceptionMessage += ", expected " + str(numFidsInList1) + " but got "
  exceptionMessage += str(numFidsInList1AfterRestore)
  raise Exception(exceptionMessage)

# fid2AfterRestore = slicer.mrmlScene.GetNodeByID("vtkMRMLMarkupsFiducialNode2")
fid2AfterRestore = slicer.mrmlScene.GetFirstNodeByName("FidNode2")
numFidsInList2AfterRestore = fid2AfterRestore.GetNumberOfMarkups()
print "After restore, list with name FidNode2 has id ", fid2AfterRestore.GetID(), " and num fids = ", numFidsInList2AfterRestore
if numFidsInList2AfterRestore != numFidsInList2:
  exceptionMessage = "After restoring list 2,  id = " + fid2AfterRestore.GetID()
  exceptionMessage += ", expected " + str(numFidsInList2) + " but got "
  exceptionMessage += str(numFidsInList2AfterRestore)
  raise Exception(exceptionMessage)

# check the displayable manager for the right number of widgets/seeds
lm = slicer.app.layoutManager()
td = lm.threeDWidget(0)
ms = vtk.vtkCollection()
td.getDisplayableManagers(ms)
fidManagerIndex = -1
for i in range(ms.GetNumberOfItems()):
  m = ms.GetItemAsObject(i)
  if m.GetClassName() == "vtkMRMLMarkupsFiducialDisplayableManager3D":
    fidManagerIndex = i
    print m.GetClassName(), fidManagerIndex
if fidManagerIndex == -1:
  exceptionMessage = "Failed to find markups fiducial displayable manager 3d!"
  raise Exception(exceptionMessage)

mfm = ms.GetItemAsObject(fidManagerIndex)

h = mfm.GetHelper()
print 'Helper = ',h

seedWidget1 = h.GetWidget(fid1AfterRestore)
rep1 = seedWidget1.GetRepresentation()
print "Seed widget 1 has number of seeds = ",rep1.GetNumberOfSeeds()
if rep1.GetNumberOfSeeds() != numFidsInList1AfterRestore:
  exceptionMessage = "After restoring list 1, expected seed widget to have "
  exceptionMessage += str(numFidsInList1AfterRestore) + " seeds, but it has "
  exceptionMessage += str(rep1.GetNumberOfSeeds())
  raise Exception(exceptionMessage)
# check positions
for s in range(numFidsInList1AfterRestore):
  seed = seedWidget1.GetSeed(s)
  handleRep = seed.GetHandleRepresentation()
  worldPos = handleRep.GetWorldPosition()
  print "seed ",s," world position = ",worldPos
  fidPos = [0.0,0.0,0.0]
  fid1AfterRestore.GetNthFiducialPosition(s,fidPos)
  xdiff = fidPos[0] - worldPos[0]
  ydiff = fidPos[1] - worldPos[1]
  zdiff = fidPos[2] - worldPos[2]
  diffTotal = xdiff + ydiff + zdiff
  if diffTotal > 0.1:
    exceptionMessage = "List1: Difference between seed position " + str(s)
    exceptionMessage += " and fiducial position totals = " + str(diffTotal)
    raise Exception(exceptionMessage)

seedWidget2 = h.GetWidget(fid2AfterRestore)
rep2 = seedWidget2.GetRepresentation()
print "Seed widget 2 has number of seeds = ",rep2.GetNumberOfSeeds()
if rep2.GetNumberOfSeeds() != numFidsInList2AfterRestore:
  exceptionMessage = "After restoring fid list 2, expected seed widget to have "
  exceptionMessage += str(numFidsInList2AfterRestore) + " seeds, but it has "
  exceptionMessage += str(rep2.GetNumberOfSeeds())
  raise Exception(exceptionMessage)
# check positions
for s in range(numFidsInList2AfterRestore):
  seed = seedWidget2.GetSeed(s)
  handleRep = seed.GetHandleRepresentation()
  worldPos = handleRep.GetWorldPosition()
  print "seed ",s," world position = ",worldPos
  fidPos = [0.0,0.0,0.0]
  fid2AfterRestore.GetNthFiducialPosition(s,fidPos)
  xdiff = fidPos[0] - worldPos[0]
  ydiff = fidPos[1] - worldPos[1]
  zdiff = fidPos[2] - worldPos[2]
  diffTotal = xdiff + ydiff + zdiff
  if diffTotal > 0.1:
    exceptionMessage = "List2: Difference between seed position " + str(s)
    exceptionMessage += " and fiducial position totals = " + str(diffTotal)
    raise Exception(exceptionMessage)

ms.RemoveAllItems()
