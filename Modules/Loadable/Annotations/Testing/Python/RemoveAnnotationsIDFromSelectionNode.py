from __future__ import print_function

# get the selection node
selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")

if selectionNode:
  print(selectionNode)
  annotClassName = "vtkMRMLAnnotationRulerNode"
  startIndex = selectionNode.PlaceNodeClassNameInList(annotClassName)
  print("Removing ", annotClassName)
  selectionNode.RemovePlaceNodeClassNameFromList(annotClassName)
  endIndex = selectionNode.PlaceNodeClassNameInList(annotClassName)
  print(selectionNode)
  print("Start index for ", annotClassName, " = ", startIndex, ", end index after removing it = ", endIndex)
  if endIndex != -1:
    raise Exception("Failed to remove annotation %s from list, end index = %s should be -1" % (annotClassName, endIndex))

  # now make one active and remove it
  annotClassName = "vtkMRMLAnnotationFiducialNode"
  selectionNode.SetActivePlaceNodeClassName(annotClassName)
  interactionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLInteractionNodeSingleton")
  interactionNode.SwitchToSinglePlaceMode()
  print("Removing", annotClassName)
  selectionNode.RemovePlaceNodeClassNameFromList(annotClassName)
  endIndex = selectionNode.PlaceNodeClassNameInList(annotClassName)
  if endIndex != -1:
    raise Exception("Failed to remove active annotation %s from list, end index = %s should be -1" % (annotClassName, endIndex))

  # re-add the ruler one
  annotClassName = "vtkMRMLAnnotationRulerNode"
  print("Adding back the ruler node")
  selectionNode.AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationRulerNode", ":/Icons/AnnotationDistanceWithArrow.png")
  endIndex = selectionNode.PlaceNodeClassNameInList(annotClassName)
  if endIndex == -1:
    raise Exception("Failed to re-add %s, end index = %s" % (annotClassName, endIndex))

