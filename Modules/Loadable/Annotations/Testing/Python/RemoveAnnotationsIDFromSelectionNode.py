
# get the selection node
selectionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLSelectionNodeSingleton")

if selectionNode:
  print selectionNode
  annotID = "vtkMRMLAnnotationRulerNode"
  startIndex = selectionNode.AnnotationIDInList(annotID)
  print "Removing ", annotID
  selectionNode.RemoveAnnotationIDFromList(annotID)
  endIndex = selectionNode.AnnotationIDInList(annotID)
  print selectionNode
  print "Start index for ", annotID, " = ", startIndex, ", end index after removing it = ", endIndex
  if endIndex != -1:
    raise Exception("Failed to remove annotation %s from list, end index = %s should be -1" % (annotID, endIndex))

  # now make one active and remove it
  annotID = "vtkMRMLAnnotationFiducialNode"
  selectionNode.SetActiveAnnotationID(annotID)
  interactionNode = slicer.mrmlScene.GetNodeByID("vtkMRMLInteractionNodeSingleton")
  interactionNode.SwitchToSinglePlaceMode()
  print "Removing", annotID
  selectionNode.RemoveAnnotationIDFromList(annotID)
  endIndex = selectionNode.AnnotationIDInList(annotID)
  if endIndex != -1:
    raise Exception("Failed to remove active annotation %s from list, end index = %s should be -1" % (annotID, endIndex))

  # re-add the ruler one
  annotID = "vtkMRMLAnnotationRulerNode"
  print "Adding back the ruler node"
  selectionNode.AddNewAnnotationIDToList("vtkMRMLAnnotationRulerNode", ":/Icons/AnnotationDistanceWithArrow.png")
  endIndex = selectionNode.AnnotationIDInList(annotID)
  if endIndex == -1:
    raise Exception("Failed to re-add %s, end index = %s" % (annotID, endIndex))

