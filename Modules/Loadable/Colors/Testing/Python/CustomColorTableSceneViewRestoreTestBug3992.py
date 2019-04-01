from __future__ import print_function
import math

# Test that a custom color table is consistent across scene view saves
# and restores

colorNode = slicer.vtkMRMLColorTableNode()
colorNode.SetName('CustomTest')
colorNode.SetHideFromEditors(0)
colorNode.SetTypeToFile()
colorNode.NamesInitialisedOff()
colorNode.SetNumberOfColors(3)
if colorNode.GetLookupTable() is not None:
  colorNode.GetLookupTable().SetTableRange(0,2)

colorNode.SetColor(0, 'zero', 0.0, 0.0, 0.0, 0.0)
colorNode.SetColor(1, 'one', 1.0, 1.0, 1.0, 1.0)
colorNode.SetColor(2, 'two', 0.5, 0.5, 0.5)

colorNode.SetColorName(0, 'zero')
colorNode.SetColorName(1, 'one')
colorNode.SetColorName(2, 'two')

colorNode.NamesInitialisedOn()

slicer.mrmlScene.AddNode(colorNode)

filePath = slicer.app.temporaryPath + "/customColorTableSceneViewRestore.ctbl"
colorStorageNode = slicer.vtkMRMLColorTableStorageNode()
colorStorageNode.SetFileName(filePath)
slicer.mrmlScene.AddNode(colorStorageNode)
colorNode.SetAndObserveStorageNodeID(colorStorageNode.GetID())

startCol2 = [0.,0.,0.,0.]
colorNode.GetColor(2, startCol2)
print("Starting color 2 =\n\t",startCol2)

sv = slicer.mrmlScene.AddNode(slicer.vtkMRMLSceneViewNode())
sv.SetName('Scene View Custom Color Test')
sv.StoreScene()

mainSceneCol2 = [0.3, 0.3, 0.3, 1.0]
colorNode.SetColor(2, mainSceneCol2[0], mainSceneCol2[1], mainSceneCol2[2], mainSceneCol2[3])
colorNode.GetColor(2, mainSceneCol2)
print('After saving the scene view, set the main scene color 2 to\n\t',mainSceneCol2)

url = slicer.app.temporaryPath + "/customColorTableSceneViewRestore.mrml"
slicer.mrmlScene.SetURL(url)
slicer.mrmlScene.Commit()
print("Saved to ",url)
# make sure it writes the color table
writeFlag = colorStorageNode.WriteData(colorNode)
if writeFlag == 0:
  print("Error writing out file ",colorStorageNode.GetFileName())


# clear out the scene and re-read from disk
slicer.mrmlScene.Clear(0)


slicer.mrmlScene.Connect()

readColorNode = slicer.util.getFirstNodeByName('CustomTest')

afterReadSceneCol2 = [0., 0., 0., 0.]
readColorNode.GetColor(2, afterReadSceneCol2)
print('After reading in the scene again, have color 2 =\n\t',afterReadSceneCol2)

readSceneView = slicer.util.getFirstNodeByName('Scene View Custom Color Test')

# Current implementation is a hack to not delete the whole color table on restore, but it also won't restore the color value to the original as it's bypassing the copy since the color table in the scene view is empty.
readSceneView.RestoreScene()

colorNodeAfterRestore =  slicer.util.getFirstNodeByName('CustomTest')
# mrmlScene.GetNodeByID("vtkMRMLColorTableNode1")

if colorNodeAfterRestore is None:
  exceptionMessage = "Unable to find vtkMRMLColorTableNode1 in scene after restore"
  raise Exception(exceptionMessage)

numColors = colorNodeAfterRestore.GetNumberOfColors()

if numColors != 3:
  exceptionMessage = "Color node doesn't have 3 colors, instead has " + str(numColors)
  raise Exception(exceptionMessage)

afterRestoreSceneCol2 = [0., 0., 0., 0.0]
colorNodeAfterRestore.GetColor(2, afterRestoreSceneCol2)
print("After restoring the scene, color 2 =\n\t", afterRestoreSceneCol2)

# the Oct'15 work around to not losing the main scene color node's colors is to not
# copy the scene view color node table values as they are all zeroes, so the expected
# difference should be 0.
# When we support having different storable node values read from disk in scene views,
# this test will fail and will need to be updated to check that afterRestoreSceneCol2
# is the same as startCol2
# Mantis issue #3992
rdiff = afterRestoreSceneCol2[0] - afterReadSceneCol2[0]
gdiff = afterRestoreSceneCol2[1] - afterReadSceneCol2[1]
bdiff = afterRestoreSceneCol2[2] - afterReadSceneCol2[2]
adiff = afterRestoreSceneCol2[3] - afterReadSceneCol2[3]
diffTotal = math.fabs(rdiff) + math.fabs(gdiff) + math.fabs(bdiff) + math.fabs(adiff)
print("Difference between colors after restored the scene and value from when it was read in from disk:\n\t", rdiff, gdiff, bdiff, adiff, "\n\tsummed absolute diff = ", diffTotal)

if diffTotal > 0.1:
  exceptionMessage = "Difference between color values total = " + str(diffTotal)
  raise Exception(exceptionMessage)
