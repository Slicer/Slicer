from __main__ import slicer
import vtk


class qSlicerMultiVolumeExplorerModuleHelper:

  @staticmethod
  def RGBtoHex(r, g, b):
    return f'#{r:02X}{g:02X}{b:02X}'

  @staticmethod
  def extractFrame(scalarVolumeNode, multiVolumeNode, frameId):
    # Extract frame from multiVolumeNode and put it into scalarVolumeNode
    # if no scalar volume given, create one
    if scalarVolumeNode is None:
      scalarVolumeNode = slicer.vtkMRMLScalarVolumeNode()
      scalarVolumeNode.SetScene(slicer.mrmlScene)
      slicer.mrmlScene.AddNode(scalarVolumeNode)

    # Extract the image data
    mvImage = multiVolumeNode.GetImageData()
    extract = vtk.vtkImageExtractComponents()
    extract.SetInputData(mvImage)
    extract.SetComponents(frameId)
    extract.Update()

    ras2ijk = vtk.vtkMatrix4x4()
    ijk2ras = vtk.vtkMatrix4x4()
    multiVolumeNode.GetRASToIJKMatrix(ras2ijk)
    multiVolumeNode.GetIJKToRASMatrix(ijk2ras)
    scalarVolumeNode.SetRASToIJKMatrix(ras2ijk)
    scalarVolumeNode.SetIJKToRASMatrix(ijk2ras)

    scalarVolumeNode.SetAndObserveImageData(extract.GetOutput())

    displayNode = scalarVolumeNode.GetDisplayNode()
    if displayNode is None:
      displayNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLScalarVolumeDisplayNode')
      displayNode.SetReferenceCount(1)
      displayNode.SetScene(slicer.mrmlScene)
      slicer.mrmlScene.AddNode(displayNode)
      displayNode.SetDefaultColorMap()
      scalarVolumeNode.SetAndObserveDisplayNodeID(displayNode.GetID())
    return scalarVolumeNode

  @staticmethod
  def SetBgFgVolumes(bg, fg):
    appLogic = slicer.app.applicationLogic()
    selectionNode = appLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(bg)
    selectionNode.SetReferenceSecondaryVolumeID(fg)
    appLogic.PropagateVolumeSelection()

  @staticmethod
  def SetBgVolume(bg):
    appLogic = slicer.app.applicationLogic()
    selectionNode = appLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(bg)
    appLogic.PropagateVolumeSelection()

  @staticmethod
  def setupChartNodeViewLayout():
    layoutNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLLayoutNode')
    layoutNodes.SetReferenceCount(layoutNodes.GetReferenceCount()-1)
    layoutNodes.InitTraversal()
    layoutNode = layoutNodes.GetNextItemAsObject()
    layoutNode.SetViewArrangement(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalQuantitativeView)

