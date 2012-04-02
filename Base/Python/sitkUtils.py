import slicer
import ctk
import vtk
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

## Copied from full_tractography_workflow.py
#
#def create_diffusion_tensor_volume_node(attach_display_node=False, dimensions=None):
#   return local_create_volume_node('DiffusionTensor', attach_display_node, dimensions=dimensions)
__sitk__VOLUME_TYPES__ = [ 'Scalar' ]
def local_create_volume_node(volume_type, nodeName="default"):
    """
    Creates a volume node and inserts it into the MRML tree
    """
    if volume_type not in __sitk__VOLUME_TYPES__:
        raise ValueError('Volume type %s is not valid' % volume_type )
    volume_node = eval('slicer.vtkMRML%sVolumeNode()' % volume_type)
    volume_node.SetName(slicer.mrmlScene.GetUniqueNameByString(nodeName))
    slicer.mrmlScene.AddNode(volume_node)
    return volume_node

def CreateNewNode( NewNodeNamePrefix , nodeType = 'vtkMRMLScalarVolumeNode'):
  """ Create a new node from scratch
  """
  node = local_create_volume_node('Scalar', NewNodeNamePrefix )
  return node.GetName()

def EnsureRegistration():
  """This is a complete hack, but attempting to read
  a dummy file with AddArchetypeVolume
  has a side effect of registering
  the MRMLImageIO file reader.
  """
  vl = slicer.modules.volumes.logic()
  volumeNode = vl.AddArchetypeVolume('_DUMMY_DOES_NOT_EXISTS__','invalidRead')

def PullFromSlicer( NodeName ):
  """ Given a slicer MRML image name, return the SimpleITK
      image object.
  """
  EnsureRegistration()
  myNodeFullITKAddress = GetSlicerITKReadWriteAddress( NodeName )
  sitkimage = sitk.ReadImage(myNodeFullITKAddress)
  return sitkimage

def PushToSlicer(sitkimage, NodeName, makeBackgroundImage=False ):
  """ Given a SimpleITK image, push it back to slicer
      for viewing
  """
  EnsureRegistration()
  newNode=CreateNewNode( NodeName )
  ## TODO: Create an empty slicer vtkMRMLScalarVolumeNode with name of "NodeName"
  ## TODO: Create Assume that the node name is now created on the SlicerScene
  myNodeFullITKAddress = GetSlicerITKReadWriteAddress( newNode )
  #print("Full Address is {0}".format(myNodeFullITKAddress))

  sitk.WriteImage(sitkimage,myNodeFullITKAddress)
  if makeBackgroundImage == True:
    appLogic = slicer.app.applicationLogic()
    selNode = appLogic.GetSelectionNode()
    volumeNode = slicer.util.getNode( newNode )
    selNode.SetReferenceActiveVolumeID(volumeNode.GetID())
    appLogic.PropagateVolumeSelection()

###############
############### The real work starts here
###############
def slicerNotes_UnitTest():
  newPointSource = sitk.Image(128,128,128,sitk.sitkFloat32)
  newPointSource[64,64,64]=1
  for sigma in range(1,12,3):
    outputName = "sigmaImage_"+str(sigma)
    smoothimage = sitk.SmoothingRecursiveGaussian(newPointSource,sigma)
    PushToSlicer(smoothimage,outputName,True)

#  To show image in external ImageJ program sitk.Show(smoothimage)
