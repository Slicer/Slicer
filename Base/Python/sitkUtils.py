import slicer
import ctk
import vtk
import SimpleITK as sitk
import warnings

__sitk__MRMLIDImageIO_Registered__ = False

def PushVolumeToSlicer(sitkimage, targetNode=None, name=None, className='vtkMRMLScalarVolumeNode'):
    """ Given a SimpleITK image, push it back to slicer for viewing

    :param targetNode: Target node that will store the image. If None then a new node will be created.
    :param className: if a new target node is created then this parameter determines node class. For label volumes, set it to vtkMRMLLabelMapVolumeNode.
    :param name: if a new target node is created then this parameter will be used as basis of node name.
      If an existing node is specified as targetNode then this value will not be used.
    """

    EnsureRegistration()

    # Create new node if needed
    if not targetNode:
        targetNode = slicer.mrmlScene.AddNewNodeByClass(className, slicer.mrmlScene.GetUniqueNameByString(name))
        targetNode.CreateDefaultDisplayNodes()

    myNodeFullITKAddress = GetSlicerITKReadWriteAddress(targetNode)
    sitk.WriteImage(sitkimage, myNodeFullITKAddress)

    return targetNode


def PullVolumeFromSlicer(nodeObjectOrName):
    """ Given a slicer MRML image node or name, return the SimpleITK
        image object.
    """
    EnsureRegistration()
    myNodeFullITKAddress = GetSlicerITKReadWriteAddress(nodeObjectOrName)
    sitkimage = sitk.ReadImage(myNodeFullITKAddress)
    return sitkimage

def GetSlicerITKReadWriteAddress(nodeObjectOrName):
    """ This function will return the ITK FileIO formatted text address
            so that the image can be read directly from the MRML scene
    """
    myNode = nodeObjectOrName if isinstance(nodeObjectOrName, slicer.vtkMRMLNode) else slicer.util.getNode(nodeObjectOrName)
    myNodeSceneAddress = myNode.GetScene().GetAddressAsString("").replace('Addr=','')
    myNodeSceneID = myNode.GetID()
    myNodeFullITKAddress = 'slicer:' + myNodeSceneAddress + '#' + myNodeSceneID
    return myNodeFullITKAddress

def EnsureRegistration():
    """Make sure MRMLIDImageIO reader is registered.
    """
    if 'MRMLIDImageIO' in sitk.ImageFileReader().GetRegisteredImageIOs():
      # already registered
      return

    # Probably this hack is not needed anymore, but it would require some work to verify this,
    # so for now just leave this here:
    # This is a complete hack, but attempting to read a dummy file with AddArchetypeVolume
    # has a side effect of registering the MRMLIDImageIO file reader.
    global __sitk__MRMLIDImageIO_Registered__
    if __sitk__MRMLIDImageIO_Registered__:
      return
    vl = slicer.modules.volumes.logic()
    volumeNode = vl.AddArchetypeVolume('_DUMMY_DOES_NOT_EXIST__','invalidRead')
    __sitk__MRMLIDImageIO_Registered__ = True

###################################################################
# Following methods in this module are all deprecated because
# they are replaced by more robust and simpler methods.
# They are kept only for temporarily for backward compatibility
# and will be removed in the future.
#
# Problem with the old methods:
#  - Referring to nodes by name is unreliable: several nodes may have the same node name;
#    users may change node name at any time; node names may be changed when added to the scene
#    if a node name is already used, etc.
#  - Old APIs are used. For example, adding of display nodes, showing volumes in certain views, adding nodes
#    to the scene have simpler, more effective methods.
#

def CloneSlicerNode( NodeName, NewNodeNamePrefix ):
    """ Create a new node in slicer by cloning
            from an existing node.
        The method is deprecated and will be removed in the future.
    """

    warnings.warn("'CloneSlicerNode' method is deprecated and will be removed in the future."+
        " Use slicer.modules.volumes.logic().CloneVolume(slicer.mrmlScene, sourceNode, newNodeNamePrefix) instead.",
        FutureWarning)

    n = slicer.util.getNode(NodeName)
    vl = slicer.modules.volumes.logic()
    newvol = vl.CloneVolume(slicer.mrmlScene,n,NewNodeNamePrefix)
    return newvol.GetName()

__sitk__VOLUME_TYPES__ = [ 'Scalar', 'LabelMap' ]

def checkVolumeNodeType(nodeType):
    """ Raise an error if the node type is not a recognized volume node
        The method is deprecated and will be removed in the future.
    """

    warnings.warn("'checkVolumeNodeType' method is deprecated and will be removed in the future.",
        FutureWarning)

    global __sitk__VOLUME_TYPES__
    volume_type = nodeType.rsplit('VolumeNode')[0].split('vtkMRML')[1]
    if volume_type not in __sitk__VOLUME_TYPES__:
        raise ValueError('Volume type %s is not valid' % volume_type )

def CreateNewVolumeNode(nodeName, nodeType='vtkMRMLScalarVolumeNode', overwrite=False):
    """ Create a new node from scratch
        The method is deprecated and will be removed in the future.
    """

    warnings.warn("'CreateNewVolumeNode' method is deprecated and will be removed in the future.",
        FutureWarning)

    scene = slicer.mrmlScene
    checkVolumeNodeType(nodeType)
    newNode = scene.CreateNodeByClass(nodeType)
    newNode.UnRegister(scene)
    AddNodeToMRMLScene(newNode, nodeName, overwrite)
    return newNode

def CreateNewDisplayNode(nodeName='default'):
    """ Create a new node from scratch
        The method is deprecated and will be removed in the future.
    """

    warnings.warn("'CreateNewVolumeNode' method is deprecated and will be removed in the future."+
        "Use volumeNode.CreateDefaultDisplayNodes() method instead.",
        FutureWarning)

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
        The method is deprecated and will be removed in the future.
    """

    warnings.warn("'removeOldMRMLNode' method is deprecated and will be removed in the future.",
        FutureWarning)

    scene = slicer.mrmlScene
    collection = scene.GetNodesByClassByName(node.GetClassName(), node.GetName())
    collection.UnRegister(scene)

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
    """ The method is deprecated and will be removed in the future.
    """

    warnings.warn("'AddNodeToMRMLScene' method is deprecated and will be removed in the future.",
        FutureWarning)

    scene = slicer.mrmlScene
    if not overwrite:
        nodeName = scene.GetUniqueNameByString(nodeName)
    newNode.SetName(nodeName)
    if overwrite:
        removeOldMRMLNode(newNode)
    scene.AddNode(newNode)


def PullFromSlicer( NodeName ):
    """ Given a slicer MRML image name, return the SimpleITK
            image object.
    The method is deprecated and will be removed in the future.
    Use PullVolumeFromSlicer method instead.
    """

    warnings.warn("'PullFromSlicer' method is deprecated and will be removed in the future."+
        " Use PullVolumeFromSlicer method instead.",
        FutureWarning)

    return PullVolumeFromSlicer(NodeName)

def PushToSlicer(sitkimage, NodeName, compositeView=0, overwrite=False):
    """ Given a SimpleITK image, push it back to slicer for viewing
    The method is deprecated and will be removed in the future.
    Use sitkUtils.PushVolumeToSlicer method and slicer.util.setSliceViewerLayers methods instead.
    ===============================================================
    Viewing options
    ---------------
    bit 0: Set as background image
    bit 1: Set as foreground image
    bit 2: Set as label image
    """

    warnings.warn("'removeOldMRMLNode' method is deprecated and will be removed in the future."+
        " Use sitkUtils.PushVolumeToSlicer method and slicer.util.setSliceViewerLayers methods instead.",
        FutureWarning)

    if compositeView not in range(3):
        raise Exception("Unknown compositeView option given: {0}. Valid values are 0 = background, 1 = foreground', or 2 = label.".format(compositeView))

    if compositeView == 2:
        newNodeClassName = 'vtkMRMLLabelMapVolumeNode'
    else:
        newNodeClassName = 'vtkMRMLScalarVolumeNode'

    newNodeBaseName = NodeName
    targetNode = None
    if overwrite:
        # reuse node if possible
        try:
            targetNode = slicer.util.getNode(NodeName)
            if targetNode.GetClassName() != newNodeClassName:
                # target node incompatible, need to create a new one
                slicer.mrmlScene.RemoveNode(targetNode)
                targetNode = None
        except slicer.util.MRMLNodeNotFoundException:
            targetNode = None

    targetNode = PushVolumeToSlicer(sitkimage, targetNode=targetNode, name=newNodeBaseName, className=newNodeClassName)

    if compositeView == 0:
        slicer.util.setSliceViewerLayers(background = targetNode)
    elif compositeView == 1:
        slicer.util.setSliceViewerLayers(foreground = targetNode)
    elif compositeView == 2:
        slicer.util.setSliceViewerLayers(label = targetNode)

    return targetNode


# Helper functions
def PushBackground(sitkImage, nodeName, overwrite=False):
    warnings.warn("'PushBackground' method is deprecated and will be removed in the future."+
        " Use sitkUtils.PushVolumeToSlicer method and slicer.util.setSliceViewerLayers methods instead.",
        FutureWarning)
    PushToSlicer(sitkImage, nodeName, 0, overwrite)

def PushForeground(sitkImage, nodeName, overwrite=False):
    warnings.warn("'PushForeground' method is deprecated and will be removed in the future."+
        " Use sitkUtils.PushVolumeToSlicer method and slicer.util.setSliceViewerLayers methods instead.",
        FutureWarning)
    PushToSlicer(sitkImage, nodeName, 1, overwrite)

def PushLabel(sitkImage, nodeName, overwrite=False):
    warnings.warn("'PushLabel' method is deprecated and will be removed in the future."+
        " Use sitkUtils.PushVolumeToSlicer method and slicer.util.setSliceViewerLayers methods instead.",
        FutureWarning)
    PushToSlicer(sitkImage, nodeName, 2, overwrite)

###############
############### Testing
###############
def slicerNotes_UnitTest():
    newPointSource = sitk.Image(128,128,128,sitk.sitkFloat32)
    newPointSource[64,64,64]=1
    for sigma in range(1,12,3):
        outputName = "sigmaImage_"+str(sigma)
        smoothimage = sitk.SmoothingRecursiveGaussian(newPointSource,sigma)
        PushToSlicer(smoothimage,outputName,True)

#    To show image in external ImageJ program sitk.Show(smoothimage)
