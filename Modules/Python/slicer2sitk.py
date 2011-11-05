import slicer
import ctk
import SimpleITK as sitk

## This assumes that you have already loaded the 'MRHead1' data from the informatics module.
def GetSlicerITKReadWriteAddress( NodeName ):
  """ This function will return the ITK FileIO formatted text address
      so that the image can be read directly from the MRML scene
  """
  #nodesList = slicer.util.getNodes('*vtkMRMLScalarVolumeNode*')
  myNode = slicer.util.getNode(NodeName)
  myNodeSceneAddress = myNode.GetScene().GetAddressAsString("").replace('Addr=','')
  myNodeSceneID = myNode.GetID()
  myNodeFullITKAddress = 'slicer:' + myNodeSceneAddress + '#' + myNodeSceneID
  return myNodeFullITKAddress

def CloneSlicerNode( NodeName, NewNodeNamePrefix ):
  """ Create a new node in slicer by cloning
      from an exising node.
  """
  n = slicer.util.getNode(NodeName)
  vl = slicer.modules.volumes.logic()
  newvol = vl.CloneVolume(slicer.mrmlScene,n,NewNodeNamePrefix)
  return newvol.GetName()


def PullFromSlicer( NodeName ):
  """ Given a slicer MRML image name, return the SimpleITK
      image object.
  """
  myNodeFullITKAddress = GetSlicerITKReadWriteAddress( NodeName )
  sitkimage = sitk.ReadImage(myNodeFullITKAddress)
  return sitkimage


def PushToSlicer(sitkimage, NodeName, makeBackgroundImage=False ):
  """ Given a SimpleITK image, push it back to slicer
      for viewing
  """
  ## TODO: Create an empty slicer vtkMRMLScalarVolumeNode with name of "NodeName"
  ## TODO: Create Assume that the node name is now created on the SlicerScene
  myNodeFullITKAddress = GetSlicerITKReadWriteAddress( NodeName )
  sitk.WriteImage(sitkimage,myNodeFullITKAddress)
  if makeBackgroundImage == True:
    appLogic = slicer.app.applicationLogic()
    selNode = appLogic.GetSelectionNode()
    volumeNode = slicer.util.getNode( NodeName )
    selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
    appLogic.PropagateVolumeSelection()

###############
############### The real work starts here
###############
def slicerNotes_UnitTest():
  for sigma in range(1,12,3):
    outputName = CloneSlicerNode('MRHead1','SmoothedImage')
    myimage = PullFromSlicer('MRHead1')
    smoothimage = sitk.SmoothingRecursiveGaussian(myimage,sigma)
    PushToSlicer(smoothimage,outputName,True)

#  To show image in external ImageJ program sitk.Show(smoothimage)

