import SimpleITK as sitk
import vtk
import slicer


def _get_lps_to_ras_matrix():
    """Get the LPS to RAS conversion matrix (diagonal matrix with -1, -1, 1)."""
    lpsToRas = vtk.vtkMatrix4x4()
    lpsToRas.Identity()
    lpsToRas.SetElement(0, 0, -1)
    lpsToRas.SetElement(1, 1, -1)
    return lpsToRas


def PushVolumeToSlicer(sitkimage, targetNode=None, name=None, className="vtkMRMLScalarVolumeNode"):
    """Given a SimpleITK image, push it back to slicer for viewing

    A deep copy of the image data is created during conversion.

    :param targetNode: Target node that will store the image. If None then a new node will be created.
    :param className: if a new target node is created then this parameter determines node class. For label volumes, set it to vtkMRMLLabelMapVolumeNode.
    :param name: if a new target node is created then this parameter will be used as basis of node name.
      If an existing node is specified as targetNode then this value will not be used.
    """
    # Create new node if needed
    if not targetNode:
        targetNode = slicer.mrmlScene.AddNewNodeByClass(className, slicer.mrmlScene.GetUniqueNameByString(name))
        targetNode.CreateDefaultDisplayNodes()

    # Set the image data (just pixel data, no spatial info)
    targetNode.SetAndObserveImageData(sitk2vtk(sitkimage))

    # Convert spatial information from LPS (SimpleITK) to RAS (Slicer)
    # Build IJK to LPS matrix
    spacing = sitkimage.GetSpacing()
    direction_lps = sitkimage.GetDirection()
    origin_lps = sitkimage.GetOrigin()

    ijkToLPS = vtk.vtkMatrix4x4()
    for row in range(3):
        for col in range(3):
            ijkToLPS.SetElement(row, col, direction_lps[row * 3 + col] * spacing[col])
        ijkToLPS.SetElement(row, 3, origin_lps[row])

    # Convert from LPS to RAS: ijkToRAS = lpsToRas * ijkToLPS
    lpsToRas = _get_lps_to_ras_matrix()
    ijkToRAS = vtk.vtkMatrix4x4()
    vtk.vtkMatrix4x4.Multiply4x4(lpsToRas, ijkToLPS, ijkToRAS)

    # Set the IJKToRAS matrix on the volume node
    targetNode.SetIJKToRASMatrix(ijkToRAS)
    targetNode.Modified()

    return targetNode


def PullVolumeFromSlicer(nodeObjectOrName):
    """Given a slicer MRML image node or name, return the SimpleITK image object."""
    targetNode = nodeObjectOrName if isinstance(nodeObjectOrName, slicer.vtkMRMLNode) else slicer.util.getNode(nodeObjectOrName)

    # Extract IJK to RAS matrix from volume node
    ijkToRAS = vtk.vtkMatrix4x4()
    targetNode.GetIJKToRASMatrix(ijkToRAS)

    # Convert from RAS to LPS: ijkToLPS = rasToLps * ijkToRAS
    rasToLps = _get_lps_to_ras_matrix()  # Same matrix works both ways (diagonal)
    ijkToLPS = vtk.vtkMatrix4x4()
    vtk.vtkMatrix4x4.Multiply4x4(rasToLps, ijkToRAS, ijkToLPS)

    # Extract origin, spacing, and direction from IJK to LPS matrix
    spacing = targetNode.GetSpacing()
    origin_lps = [ijkToLPS.GetElement(i, 3) for i in range(3)]
    direction_lps = [ijkToLPS.GetElement(row, col) / spacing[col] for row in range(3) for col in range(3)]

    return vtk2sitk(targetNode.GetImageData(), origin=origin_lps, spacing=spacing, direction=direction_lps)


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
    """Deprecated: This function always returns True.

    PushVolumeToSlicer and PullVolumeFromSlicer now use numpy-based conversions
    for fast in-memory ITK image transfer, independent of MRMLIDImageIO availability
    previously made available in a customized SimpleITK version.
    """
    return True


def vtk2sitk(vtkimg, origin=None, spacing=None, direction=None):
    """Convert VTK image to SimpleITK image.

    :param origin: Origin (if None, extracted from vtkimg)
    :param spacing: Spacing (if None, extracted from vtkimg)
    :param direction: Direction matrix as flat list (if None, extracted from vtkimg)
    """
    import vtk.util.numpy_support as vtknp

    # Convert pixel data via numpy
    npdata = vtknp.vtk_to_numpy(vtkimg.GetPointData().GetScalars())
    dims = list(vtkimg.GetDimensions())
    dims.reverse()
    npdata.shape = tuple(dims)

    # Create SimpleITK image and set spatial properties
    sitkimg = sitk.GetImageFromArray(npdata)
    sitkimg.SetSpacing(spacing if spacing is not None else vtkimg.GetSpacing())

    if direction is not None:
        sitkimg.SetDirection(direction)
    else:
        vtk_dir = vtkimg.GetDirectionMatrix()
        sitkimg.SetDirection([vtk_dir.GetElement(row, col) for row in range(3) for col in range(3)])

    sitkimg.SetOrigin(origin if origin is not None else vtkimg.GetOrigin())
    return sitkimg


def sitk2vtk(img, deep=1):
    """Convert SimpleITK image to VTK image suitable for MRML volume nodes.

    Input: Image (SimpleITK.Image), deep copy (integer bool, default True)

    Note: Returns vtkImageData with origin=(0,0,0), spacing=(1,1,1), and identity direction.
    Spatial information should be set on the MRML volume node via IJKToRASMatrix.
    """
    import vtk.util.numpy_support as vtknp

    # Convert pixel data via numpy
    npdata = sitk.GetArrayFromImage(img)
    size = list(img.GetSize())

    # Ensure 3D dimensions
    if len(size) == 2:
        size.append(1)

    # Create VTK image with identity transform (required by MRML)
    vtk_image = vtk.vtkImageData()
    vtk_image.SetDimensions(size)
    vtk_image.SetSpacing(1.0, 1.0, 1.0)
    vtk_image.SetOrigin(0.0, 0.0, 0.0)

    # Set pixel data
    depth_array = vtknp.numpy_to_vtk(npdata.ravel(), deep=deep)
    depth_array.SetNumberOfComponents(img.GetNumberOfComponentsPerPixel())
    vtk_image.GetPointData().SetScalars(depth_array)

    return vtk_image
