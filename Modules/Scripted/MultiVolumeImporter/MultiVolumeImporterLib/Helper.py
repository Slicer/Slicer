from __main__ import slicer

class Helper:

  @staticmethod
  def SetBgFgVolumes(bgID, fgID):
    appLogic = slicer.app.applicationLogic()
    selectionNode = appLogic.GetSelectionNode()
    if bgID is not None:
      selectionNode.SetReferenceActiveVolumeID(bgID)
    if fgID is not None:
      selectionNode.SetReferenceSecondaryVolumeID(fgID)
    appLogic.PropagateVolumeSelection()
