import SimpleITK as sitk

import slicer

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
    myNodeSceneAddress = myNode.GetScene().GetAddressAsString("").replace('Addr=', '')
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
    volumeNode = vl.AddArchetypeVolume('_DUMMY_DOES_NOT_EXIST__', 'invalidRead')
    __sitk__MRMLIDImageIO_Registered__ = True
