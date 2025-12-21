import SimpleITK as sitk
import os
import vtk
from vtk.util import numpy_support
import vtk.util.numpy_support as vtknp
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


def vtk2sitk(vtkimg, debug=False):
    """Takes a VTK image, returns a SimpleITK image."""
    sd = vtkimg.GetPointData().GetScalars()
    npdata = vtknp.vtk_to_numpy(sd)

    dims = list(vtkimg.GetDimensions())
    origin = vtkimg.GetOrigin()
    spacing = vtkimg.GetSpacing()

    if debug:
        print("dims:", dims)
        print("origin:", origin)
        print("spacing:", spacing)

        print("numpy type:", npdata.dtype)
        print("numpy shape:", npdata.shape)

    dims.reverse()
    npdata.shape = tuple(dims)
    if debug:
        print("new shape:", npdata.shape)
    sitkimg = sitk.GetImageFromArray(npdata)
    sitkimg.SetSpacing(spacing)
    sitkimg.SetOrigin(origin)

    if vtk.vtkVersion.GetVTKMajorVersion() >= 9:
        direction = vtkimg.GetDirectionMatrix()
        d = []
        for y in range(3):
            for x in range(3):
                d.append(direction.GetElement(y, x))
        sitkimg.SetDirection(d)
    return sitkimg


def sitk2vtk(img, debugOn=False):
    """
    Convert a SimpleITK image to a VTK image, via numpy.

    Written by David T. Chen from the National Institute of Allergy
    and Infectious Diseases, dchen@mail.nih.gov.
    It is covered by the Apache License, Version 2.0:
    http://www.apache.org/licenses/LICENSE-2.0
    """

    size = list(img.GetSize())
    origin = list(img.GetOrigin())
    spacing = list(img.GetSpacing())
    ncomp = img.GetNumberOfComponentsPerPixel()
    direction = img.GetDirection()

    # there doesn't seem to be a way to specify the image orientation in VTK

    # convert the SimpleITK image to a numpy array
    i2 = sitk.GetArrayFromImage(img)
    if debugOn:
        i2_string = i2.tostring()
        print("data string address inside sitk2vtk", hex(id(i2_string)))

    vtk_image = vtk.vtkImageData()

    # VTK expects 3-dimensional parameters
    if len(size) == 2:
        size.append(1)

    if len(origin) == 2:
        origin.append(0.0)

    if len(spacing) == 2:
        spacing.append(spacing[0])

    if len(direction) == 4:
        direction = [
            direction[0],
            direction[1],
            0.0,
            direction[2],
            direction[3],
            0.0,
            0.0,
            0.0,
            1.0,
        ]

    vtk_image.SetDimensions(size)
    vtk_image.SetSpacing(spacing)
    vtk_image.SetOrigin(origin)
    vtk_image.SetExtent(0, size[0] - 1, 0, size[1] - 1, 0, size[2] - 1)

    if vtk.vtkVersion.GetVTKMajorVersion() < 9:
        print("Warning: VTK version <9.  No direction matrix.")
    else:
        vtk_image.SetDirectionMatrix(direction)

    # depth_array = numpy_support.numpy_to_vtk(i2.ravel(), deep=True,
    #                                          array_type = vtktype)
    depth_array = numpy_support.numpy_to_vtk(i2.ravel())
    depth_array.SetNumberOfComponents(ncomp)
    vtk_image.GetPointData().SetScalars(depth_array)

    vtk_image.Modified()
    #
    if debugOn:
        print("Volume object inside sitk2vtk")
        print(vtk_image)
        #        print("type = ", vtktype)
        print("num components = ", ncomp)
        print(size)
        print(origin)
        print(spacing)
        print(vtk_image.GetScalarComponentAsFloat(0, 0, 0, 0))

    return vtk_image
