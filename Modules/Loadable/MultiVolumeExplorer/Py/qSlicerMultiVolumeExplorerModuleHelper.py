# slicer imports
from __main__ import vtk, slicer, tcl

class qSlicerMultiVolumeExplorerModuleHelper( object ):
  '''
  classdocs
  '''

  @staticmethod
  def SetBgFgVolumes(bg, fg):
    appLogic = slicer.app.applicationLogic()
    selectionNode = appLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID(bg)
    selectionNode.SetReferenceSecondaryVolumeID(fg)
    appLogic.PropagateVolumeSelection()

  @staticmethod
  def RotateToVolumePlanes():
    # AF TODO: check with Steve if this has any undesired consequences
    tcl('EffectSWidget::RotateToVolumePlanes')

