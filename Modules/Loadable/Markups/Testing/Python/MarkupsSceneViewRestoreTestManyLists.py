from __future__ import print_function

# Test restoring a scene with multiple lists with different number
# of fiducials

# first fiducial list
fidNode1 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode", "FidNode1")
fidNode1.CreateDefaultDisplayNodes()
coords = [0.0, 0.0, 0.0]
numFidsInList1 = 5
for i in range(numFidsInList1):
  fidNode1.AddFiducialFromArray(coords)
  coords[0] += 1.0
  coords[1] += 2.0
  coords[2] += 1.0

# second fiducial list
fidNode2 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode", "FidNode2")
fidNode2.CreateDefaultDisplayNodes()
numFidsInList2 = 10
for i in range(numFidsInList2):
  fidNode2.AddFiducialFromArray(coords)
  coords[0] += 1.0
  coords[1] += 1.0
  coords[2] += 3.0

# Create scene view
numFidNodesBeforeStore = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialNode')
sv = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSceneViewNode")
sv.StoreScene()

# add a third list that will get removed on restore
# second fiducial list
fidNode3 = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsFiducialNode", "FidNode3")
fidNode3.CreateDefaultDisplayNodes()
numFidsInList3 = 2
for i in range(numFidsInList3):
  fidNode3.AddFiducialFromArray(coords)
  coords[0] += 1.0
  coords[1] += 2.0
  coords[2] += 3.0

# Restore scene view
sv.RestoreScene()

numFidNodesAfterRestore = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLMarkupsFiducialNode')
if numFidNodesAfterRestore != numFidNodesBeforeStore:
  print("After restoring the scene, expected ", numFidNodesBeforeStore, " fiducial nodes, but have ", numFidNodesAfterRestore)
  exceptionMessage = "After restoring the scene, expected " + str(numFidNodesBeforeStore) + " fiducial nodes, but have " + str(numFidNodesAfterRestore)
  raise Exception(exceptionMessage)

fid1AfterRestore = slicer.mrmlScene.GetFirstNodeByName("FidNode1")
numFidsInList1AfterRestore = fid1AfterRestore.GetNumberOfMarkups()
print("After restore, list with name FidNode1 has id ", fid1AfterRestore.GetID(), " and num fids = ", numFidsInList1AfterRestore)
if numFidsInList1AfterRestore != numFidsInList1:
  exceptionMessage = "After restoring list 1, id = " + fid1AfterRestore.GetID()
  exceptionMessage += ", expected " + str(numFidsInList1) + " but got "
  exceptionMessage += str(numFidsInList1AfterRestore)
  raise Exception(exceptionMessage)

fid2AfterRestore = slicer.mrmlScene.GetFirstNodeByName("FidNode2")
numFidsInList2AfterRestore = fid2AfterRestore.GetNumberOfMarkups()
print("After restore, list with name FidNode2 has id ", fid2AfterRestore.GetID(), " and num fids = ", numFidsInList2AfterRestore)
if numFidsInList2AfterRestore != numFidsInList2:
  exceptionMessage = "After restoring list 2,  id = " + fid2AfterRestore.GetID()
  exceptionMessage += ", expected " + str(numFidsInList2) + " but got "
  exceptionMessage += str(numFidsInList2AfterRestore)
  raise Exception(exceptionMessage)

# check the displayable manager for the right number of widgets/seeds
lm = slicer.app.layoutManager()
td = lm.threeDWidget(0)
mfm = td.threeDView().displayableManagerByClassName("vtkMRMLMarkupsDisplayableManager")
h = mfm.GetHelper()
print('Helper = ',h)

# Markups widget classes are not imported into Python by default
import vtkSlicerMarkupsModuleVTKWidgetsPython

for markupsNode in [fid1AfterRestore, fid2AfterRestore]:
  markupsWidget = h.GetWidget(markupsNode)
  rep = markupsWidget.GetRepresentation()
  controlPointsPoly = rep.GetControlPointsPolyData(rep.Selected)
  numberOfControlPoints = controlPointsPoly.GetNumberOfPoints()
  print("Markups widget {0} has number of control points = {1}".format(markupsNode.GetName(), numberOfControlPoints))
  if numberOfControlPoints != markupsNode.GetNumberOfControlPoints():
    exceptionMessage = "After restoring " + markupsNode.GetName() + ", expected widget to have "
    exceptionMessage += str(markupsNode.GetNumberOfControlPoints()) + " points, but it has "
    exceptionMessage += str(numberOfControlPoints)
    raise Exception(exceptionMessage)
  # check positions
  for s in range(markupsNode.GetNumberOfControlPoints()):
    worldPos = controlPointsPoly.GetPoint(s)
    print("control point ",s," world position = ",worldPos)
    fidPos = [0.0,0.0,0.0]
    markupsNode.GetNthFiducialPosition(s,fidPos)
    xdiff = fidPos[0] - worldPos[0]
    ydiff = fidPos[1] - worldPos[1]
    zdiff = fidPos[2] - worldPos[2]
    diffTotal = xdiff + ydiff + zdiff
    if diffTotal > 0.1:
      exceptionMessage = markupsNode.GetName() + ": Difference between control point position " + str(s)
      exceptionMessage += " and representation point position totals = " + str(diffTotal)
      raise Exception(exceptionMessage)
  # Release reference to VTK widget, otherwise application could crash on exit
  del markupsWidget
