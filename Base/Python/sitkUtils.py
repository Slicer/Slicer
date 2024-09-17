import SimpleITK as sitk
import os
import slicer

__sitk__MRMLIDImageIO_Missing_Reported__ = False


def PushVolumeToSlicer(sitkimage, targetNode=None, name=None, className="vtkMRMLScalarVolumeNode"):
    """Given a SimpleITK image, push it back to slicer for viewing

    :param targetNode: Target node that will store the image. If None then a new node will be created.
    :param className: if a new target node is created then this parameter determines node class. For label volumes, set it to vtkMRMLLabelMapVolumeNode.
    :param name: if a new target node is created then this parameter will be used as basis of node name.
      If an existing node is specified as targetNode then this value will not be used.
    """

    # Create new node if needed
    if not targetNode:
        targetNode = slicer.mrmlScene.AddNewNodeByClass(className, slicer.mrmlScene.GetUniqueNameByString(name))
        targetNode.CreateDefaultDisplayNodes()

    useMRMLIDImageIO = IsMRMLIDImageIOAvailable()
    if useMRMLIDImageIO:
        # Use direct memory transfer
        myNodeFullITKAddress = GetSlicerITKReadWriteAddress(targetNode)
        sitk.WriteImage(sitkimage, myNodeFullITKAddress)
    else:
        # Use file transfer (less efficient, but works in all cases).
        # Simple VTK/SimpleITK image conversion would not work, as coordinate system needs to be converted between LPS and RAS.
        storageNode, tempFileName = _addDefaultStorageNode(targetNode)
        sitk.WriteImage(sitkimage, tempFileName)
        storageNode.ReadData(targetNode, True)
        slicer.mrmlScene.RemoveNode(storageNode)
        os.remove(tempFileName)

    return targetNode


def PullVolumeFromSlicer(nodeObjectOrName):
    """Given a slicer MRML image node or name, return the SimpleITK
    image object.
    """
    useMRMLIDImageIO = IsMRMLIDImageIOAvailable()
    if useMRMLIDImageIO:
        # Use direct memory transfer
        myNodeFullITKAddress = GetSlicerITKReadWriteAddress(nodeObjectOrName)
        sitkimage = sitk.ReadImage(myNodeFullITKAddress)
    else:
        # Use file transfer (less efficient, but works in all cases).
        # Simple VTK/SimpleITK image conversion would not work, as coordinate system needs to be converted between LPS and RAS.
        targetNode = nodeObjectOrName if isinstance(nodeObjectOrName, slicer.vtkMRMLNode) else slicer.util.getNode(nodeObjectOrName)
        storageNode, tempFileName = _addDefaultStorageNode(targetNode)
        storageNode.WriteData(targetNode)
        sitkimage = sitk.ReadImage(tempFileName)
        slicer.mrmlScene.RemoveNode(storageNode)
        os.remove(tempFileName)

    return sitkimage


def GetSlicerITKReadWriteAddress(nodeObjectOrName):
    """This function will return the ITK FileIO formatted text address
    so that the image can be read directly from the MRML scene
    """
    myNode = nodeObjectOrName if isinstance(nodeObjectOrName, slicer.vtkMRMLNode) else slicer.util.getNode(nodeObjectOrName)
    myNodeSceneAddress = myNode.GetScene().GetAddressAsString("").replace("Addr=", "")
    myNodeSceneID = myNode.GetID()
    myNodeFullITKAddress = "slicer:" + myNodeSceneAddress + "#" + myNodeSceneID
    return myNodeFullITKAddress


def IsMRMLIDImageIOAvailable():
    """Determine if MRMLIDImageIO (fast in-memory ITK image transfer) is available.
    If not available then report the error (only once).
    """
    if "MRMLIDImageIO" in sitk.ImageFileReader().GetRegisteredImageIOs():
        # MRMLIDImageIO is available
        return True

    global __sitk__MRMLIDImageIO_Missing_Reported__
    if not __sitk__MRMLIDImageIO_Missing_Reported__:
        import logging
        logging.error(
            "MRMLIDImageIO is not available, SimpleITK image transfer speed will be slower."
            " Probably an extension replaced SimpleITK version that was bundled with Slicer."
            f" Current SimpleITK version: {sitk.__version__}")
        __sitk__MRMLIDImageIO_Missing_Reported__ = True

    return False


def _addDefaultStorageNode(targetNode):
    originalStorageNode = targetNode.GetStorageNode()
    if originalStorageNode:
        storageNode = slicer.mrmlScene.AddNewNodeByClass(originalStorageNode.GetClassName(), "__tmp__" + originalStorageNode.GetName())
    else:
        storageNode = targetNode.CreateDefaultStorageNode()
        storageNode.UnRegister(None)
        slicer.mrmlScene.AddNode(storageNode)
    import os, uuid
    tempFileName = os.path.join(slicer.app.temporaryPath, str(uuid.uuid1())) + ".nrrd"
    storageNode.SetFileName(tempFileName)
    storageNode.UseCompressionOff()
    return storageNode, tempFileName
