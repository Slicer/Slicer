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

__sitk__VOLUME_TYPES__ = [ 'Scalar', 'LabelMap' ]

def checkVolumeNodeType(nodeType):
    """ Raise an error if the node type is not a recognized volume node
    """
    volume_type = nodeType.rsplit('VolumeNode')[0].split('vtkMRML')[1]
    if volume_type not in __sitk__VOLUME_TYPES__:
        raise ValueError('Volume type %s is not valid' % volume_type )

def CreateNewVolumeNode(nodeName, nodeType='vtkMRMLScalarVolumeNode', overwrite=False):
    """ Create a new node from scratch
    """
    scene = slicer.mrmlScene
    checkVolumeNodeType(nodeType)
    newNode = scene.CreateNodeByClass(nodeType)
    newNode.UnRegister(scene)
    AddNodeToMRMLScene(newNode, nodeName, overwrite)
    return newNode

def CreateNewDisplayNode(nodeName='default'):
    """ Create a new node from scratch
    """
    scene = slicer.mrmlScene
    nodeType='vtkMRMLScalarVolumeDisplayNode'
    newNode = scene.CreateNodeByClass(nodeType)
    newNode.UnRegister(scene)
    AddNodeToMRMLScene(newNode, nodeName + '_Display', False)
    colorLogic = slicer.modules.colors.logic()
    newNode.SetAndObserveColorNodeID(colorLogic.GetDefaultVolumeColorNodeID())
    return newNode

def removeOldMRMLNode(node):
    """ Overwrite a MRML node with the same name and class as the given node
    """
    scene = slicer.mrmlScene
    collection = scene.GetNodesByClassByName(node.GetClassName(), node.GetName())
    if collection.GetNumberOfItems() == 1:
        oldNode = collection.GetItemAsObject(0)
        scene.RemoveNode(oldNode)
    elif collection.GetNumberOfItems() > 1:
        # This should never happen
        raise Exception("Too many existing nodes in MRML Scene: Unable to determine node to replace!")
    else:
        # There was no node to overwrite
        return 1
    return 0

def AddNodeToMRMLScene(newNode, nodeName='default', overwrite=False):
    scene = slicer.mrmlScene
    if not overwrite:
        nodeName = scene.GetUniqueNameByString(nodeName)
    newNode.SetName(nodeName)
    if overwrite:
        removeOldMRMLNode(newNode)
    scene.AddNode(newNode)

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

def PushToSlicer(sitkimage, NodeName, compositeView=0, overwrite=False):
    """ Given a SimpleITK image, push it back to slicer for viewing
    ===============================================================
    Viewing options
    ---------------
    bit 0: Set as background image
    bit 1: Set as foreground image
    bit 2: Set as label image
    """
    from sitkUtils import EnsureRegistration, CreateNewVolumeNode, CreateNewDisplayNode, GetSlicerITKReadWriteAddress
    scene = slicer.mrmlScene
    compositeViewMap = {0:'background',
                        1:'foreground',
                        2:'label'}
    if compositeView in compositeViewMap.keys():
        imageType = compositeViewMap[compositeView]
    else:
        raise Exception('Unknown view option given: {0}. See help'.format(compositeView))
    EnsureRegistration()
    if imageType == 'label':
        newNode = CreateNewVolumeNode(NodeName,'vtkMRMLLabelMapVolumeNode', overwrite)
    else:
        newNode = CreateNewVolumeNode(NodeName,'vtkMRMLScalarVolumeNode', overwrite)

    if not overwrite:
        newDisplayNode = CreateNewDisplayNode(NodeName)
    else:
        nodeCollection = scene.GetNodesByClassByName('vtkMRMLScalarVolumeDisplayNode', NodeName + '_Display')
        count = nodeCollection.GetNumberOfItems()
        if count == 0:
            newDisplayNode = CreateNewDisplayNode(NodeName)
        elif count == 1:
            newDisplayNode = nodeCollection.GetItemAsObject(0)
        else:
            Exception( "Too many display nodes for %s!" % NodeName )
    newNode.SetAndObserveDisplayNodeID(newDisplayNode.GetID())
    myNodeFullITKAddress = GetSlicerITKReadWriteAddress(newNode.GetName())
    sitk.WriteImage(sitkimage, myNodeFullITKAddress)
    writtenNode = slicer.util.getNode(newNode.GetName())
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    if imageType == 'foreground':
        selectionNode.SetReferenceSecondaryVolumeID(writtenNode.GetID())
    elif imageType == 'background':
        selectionNode.SetReferenceActiveVolumeID(writtenNode.GetID())
    elif imageType == 'label':
        selectionNode.SetReferenceActiveLabelVolumeID(writtenNode.GetID())

    applicationLogic = slicer.app.applicationLogic()
    applicationLogic.PropagateVolumeSelection(0)

# Helper functions
def PushBackground(sitkImage, nodeName, overwrite=False):
    PushToSlicer(sitkImage, nodeName, 0, overwrite)

def PushForeground(sitkImage, nodeName, overwrite=False):
    PushToSlicer(sitkImage, nodeName, 1, overwrite)

def PushLabel(sitkImage, nodeName, overwrite=False):
    PushToSlicer(sitkImage, nodeName, 2, overwrite)

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

#    To show image in external ImageJ program sitk.Show(smoothimage)
