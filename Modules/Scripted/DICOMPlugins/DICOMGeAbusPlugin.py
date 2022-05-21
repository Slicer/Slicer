import logging

import numpy
import pydicom as dicom
import vtk
import vtk.util.numpy_support

import slicer

from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable


#
# This is the plugin to handle translation of DICOM objects
# that can be represented as multivolume objects
# from DICOM files into MRML nodes.  It follows the DICOM module's
# plugin architecture.
#

class DICOMGeAbusPluginClass(DICOMPlugin):
    """ Image loader plugin for GE Invenia
    ABUS (automated breast ultrasound) images.
    """

    def __init__(self):
        super().__init__()
        self.loadType = "GE ABUS"

        self.tags['sopClassUID'] = "0008,0016"
        self.tags['seriesNumber'] = "0020,0011"
        self.tags['seriesDescription'] = "0008,103E"
        self.tags['instanceNumber'] = "0020,0013"
        self.tags['manufacturerModelName'] = "0008,1090"

        # Accepted private creator identifications
        self.privateCreators = ["U-Systems", "General Electric Company 01"]

    def examine(self, fileLists):
        """ Returns a list of DICOMLoadable instances
        corresponding to ways of interpreting the
        fileLists parameter.
        """
        loadables = []
        for files in fileLists:
            loadables += self.examineFiles(files)

        return loadables

    def examineFiles(self, files):
        """ Returns a list of DICOMLoadable instances
        corresponding to ways of interpreting the
        files parameter.
        """

        detailedLogging = self.isDetailedLogging()

        supportedSOPClassUIDs = [
            '1.2.840.10008.5.1.4.1.1.3.1',  # Ultrasound Multiframe Image Storage
        ]

        loadables = []

        for filePath in files:
            # Quick check of SOP class UID without parsing the file...
            try:
                sopClassUID = slicer.dicomDatabase.fileValue(filePath, self.tags['sopClassUID'])
                if not (sopClassUID in supportedSOPClassUIDs):
                    # Unsupported class
                    continue

                manufacturerModelName = slicer.dicomDatabase.fileValue(filePath, self.tags['manufacturerModelName'])
                if manufacturerModelName != "Invenia":
                    if detailedLogging:
                        logging.debug("ManufacturerModelName is not Invenia, the series will not be considered as an ABUS image")
                    continue

            except Exception as e:
                # Quick check could not be completed (probably Slicer DICOM database is not initialized).
                # No problem, we'll try to parse the file and check the SOP class UID then.
                pass

            try:
                ds = dicom.read_file(filePath, stop_before_pixels=True)
            except Exception as e:
                logging.debug(f"Failed to parse DICOM file: {str(e)}")
                continue

            # check if probeCurvatureRadius is available
            probeCurvatureRadiusFound = False
            for privateCreator in self.privateCreators:
                if self.findPrivateTag(ds, 0x0021, 0x40, privateCreator):
                    probeCurvatureRadiusFound = True
                    break

            if not probeCurvatureRadiusFound:
                if detailedLogging:
                    logging.debug("Probe curvature radius is not found, the series will not be considered as an ABUS image")
                continue

            name = ''
            if hasattr(ds, 'SeriesNumber') and ds.SeriesNumber:
                name = f'{ds.SeriesNumber}:'
            if hasattr(ds, 'Modality') and ds.Modality:
                name = f'{name} {ds.Modality}'
            if hasattr(ds, 'SeriesDescription') and ds.SeriesDescription:
                name = f'{name} {ds.SeriesDescription}'
            if hasattr(ds, 'InstanceNumber') and ds.InstanceNumber:
                name = f'{name} [{ds.InstanceNumber}]'

            loadable = DICOMLoadable()
            loadable.files = [filePath]
            loadable.name = name.strip()  # remove leading and trailing spaces, if any
            loadable.tooltip = "GE Invenia ABUS"
            loadable.warning = "Loading of this image type is experimental. Please verify image size and orientation and report any problem is found."
            loadable.selected = True
            loadable.confidence = 0.9  # this has to be higher than 0.7 (ultrasound sequence)

            # Add to loadables list
            loadables.append(loadable)

        return loadables

    def getMetadata(self, filePath):
        try:
            ds = dicom.read_file(filePath, stop_before_pixels=True)
        except Exception as e:
            raise ValueError(f"Failed to parse DICOM file: {str(e)}")

        fieldsInfo = {
            'NipplePosition': {'group': 0x0021, 'element': 0x20, 'private': True, 'required': False},
            'FirstElementPosition': {'group': 0x0021, 'element': 0x21, 'private': True, 'required': False},
            'CurvatureRadiusProbe': {'group': 0x0021, 'element': 0x40, 'private': True, 'required': True},
            'CurvatureRadiusTrack': {'group': 0x0021, 'element': 0x41, 'private': True, 'required': True},
            'LineDensity': {'group': 0x0021, 'element': 0x62, 'private': True, 'required': False},
            'ScanDepthCm': {'group': 0x0021, 'element': 0x63, 'private': True, 'required': True},
            'SpacingBetweenSlices': {'group': 0x0018, 'element': 0x0088, 'private': False, 'required': True},
            'PixelSpacing': {'group': 0x0028, 'element': 0x0030, 'private': False, 'required': True},
        }

        fieldValues = {}
        for fieldName in fieldsInfo:
            fieldInfo = fieldsInfo[fieldName]
            if fieldInfo['private']:
                for privateCreator in self.privateCreators:
                    tag = self.findPrivateTag(ds, fieldInfo['group'], fieldInfo['element'], privateCreator)
                    if tag:
                        break
            else:
                tag = dicom.tag.Tag(fieldInfo['group'], fieldInfo['element'])
            if tag:
                fieldValues[fieldName] = ds[tag].value

        # Make sure all mandatory fields are found
        for fieldName in fieldsInfo:
            fieldInfo = fieldsInfo[fieldName]
            if not fieldInfo['required']:
                continue
            if fieldName not in fieldValues:
                raise ValueError(f"Mandatory field {fieldName} was not found")

        return fieldValues

    def load(self, loadable):
        """Load the selection
        """

        filePath = loadable.files[0]
        metadata = self.getMetadata(filePath)

        import vtkITK
        reader = vtkITK.vtkITKArchetypeImageSeriesScalarReader()
        reader.SetArchetype(filePath)
        reader.AddFileName(filePath)
        reader.SetSingleFile(True)
        reader.SetOutputScalarTypeToNative()
        reader.SetDesiredCoordinateOrientationToNative()
        reader.SetUseNativeOriginOn()
        # GDCM is not particularly better in this than DCMTK, we just select one explicitly
        # so that we know which one is used
        reader.SetDICOMImageIOApproachToGDCM()
        reader.Update()
        imageData = reader.GetOutput()

        if reader.GetErrorCode() != vtk.vtkErrorCode.NoError:
            errorString = vtk.vtkErrorCode.GetStringFromErrorCode(reader.GetErrorCode())
            raise ValueError(
                f"Could not read image {loadable.name} from file {filePath}. Error is: {errorString}")

        # Image origin and spacing is stored in IJK to RAS matrix
        imageData.SetSpacing(1.0, 1.0, 1.0)
        imageData.SetOrigin(0.0, 0.0, 0.0)

        volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode", slicer.mrmlScene.GenerateUniqueName(loadable.name))

        # I axis: scanline index (lateralSpacing)
        # J axis: sound propagation (axialSpacing)
        # K axis: slice (sliceSpacing)
        lateralSpacing = metadata['PixelSpacing'][1]
        axialSpacing = metadata['PixelSpacing'][0]
        sliceSpacing = metadata['SpacingBetweenSlices']

        ijkToRas = vtk.vtkMatrix4x4()
        ijkToRas.SetElement(0, 0, -1)
        ijkToRas.SetElement(1, 1, -1)  # so that J axis points toward posterior
        volumeNode.SetIJKToRASMatrix(ijkToRas)
        volumeNode.SetSpacing(lateralSpacing, axialSpacing, sliceSpacing)
        extent = imageData.GetExtent()
        volumeNode.SetOrigin((extent[1] - extent[0] + 1) * 0.5 * lateralSpacing, 0, -(extent[5] - extent[2] + 1) * 0.5 * sliceSpacing)
        volumeNode.SetAndObserveImageData(imageData)

        # Apply scan conversion transform
        acquisitionTransform = self.createAcquisitionTransform(volumeNode, metadata)
        volumeNode.SetAndObserveTransformNodeID(acquisitionTransform.GetID())

        # Create Subject hierarchy nodes for the loaded series
        self.addSeriesInSubjectHierarchy(loadable, volumeNode)

        # Place transform in the same subject hierarchy folder as the volume node
        shNode = slicer.vtkMRMLSubjectHierarchyNode.GetSubjectHierarchyNode(slicer.mrmlScene)
        volumeParentItemId = shNode.GetItemParent(shNode.GetItemByDataNode(volumeNode))
        shNode.SetItemParent(shNode.GetItemByDataNode(acquisitionTransform), volumeParentItemId)

        # Show in slice views
        selectionNode = slicer.app.applicationLogic().GetSelectionNode()
        selectionNode.SetReferenceActiveVolumeID(volumeNode.GetID())
        slicer.app.applicationLogic().PropagateVolumeSelection(1)

        return volumeNode

    def createAcquisitionTransform(self, volumeNode, metadata):

        # Creates transform that applies scan conversion transform
        probeRadius = metadata['CurvatureRadiusProbe']
        trackRadius = metadata['CurvatureRadiusTrack']
        if trackRadius != 0.0:
            raise ValueError(f"Curvature Radius (Track) is {trackRadius}. Currently, only volume with zero radius can be imported.")

        # Create a sampling grid for the transform
        import numpy as np
        spacing = np.array(volumeNode.GetSpacing())
        averageSpacing = (spacing[0] + spacing[1] + spacing[2]) / 3.0
        voxelsPerTransformControlPoint = 20  # the transform is changing smoothly, so we don't need to add too many control points
        gridSpacingMm = averageSpacing * voxelsPerTransformControlPoint
        gridSpacingVoxel = np.floor(gridSpacingMm / spacing).astype(int)
        gridAxesIJK = []
        imageData = volumeNode.GetImageData()
        extent = imageData.GetExtent()
        for axis in range(3):
            gridAxesIJK.append(list(range(extent[axis * 2], extent[axis * 2 + 1] + gridSpacingVoxel[axis], gridSpacingVoxel[axis])))
        samplingPoints_shape = [len(gridAxesIJK[0]), len(gridAxesIJK[1]), len(gridAxesIJK[2]), 3]

        # create a grid transform with one vector at the corner of each slice
        # the transform is in the same space and orientation as the volume node
        import vtk
        gridImage = vtk.vtkImageData()
        gridImage.SetOrigin(*volumeNode.GetOrigin())
        gridImage.SetDimensions(samplingPoints_shape[:3])
        gridImage.SetSpacing(gridSpacingVoxel[0] * spacing[0], gridSpacingVoxel[1] * spacing[1], gridSpacingVoxel[2] * spacing[2])
        gridImage.AllocateScalars(vtk.VTK_DOUBLE, 3)
        transform = slicer.vtkOrientedGridTransform()
        directionMatrix = vtk.vtkMatrix4x4()
        volumeNode.GetIJKToRASDirectionMatrix(directionMatrix)
        transform.SetGridDirectionMatrix(directionMatrix)
        transform.SetDisplacementGridData(gridImage)

        # create the grid transform node
        gridTransform = slicer.vtkMRMLGridTransformNode()
        gridTransform.SetName(slicer.mrmlScene.GenerateUniqueName(volumeNode.GetName() + ' acquisition transform'))
        slicer.mrmlScene.AddNode(gridTransform)
        gridTransform.SetAndObserveTransformToParent(transform)

        # populate the grid so that each corner of each slice
        # is mapped from the source corner to the target corner

        nshape = tuple(reversed(gridImage.GetDimensions()))
        nshape = nshape + (3,)
        displacements = vtk.util.numpy_support.vtk_to_numpy(gridImage.GetPointData().GetScalars()).reshape(nshape)

        # Get displacements
        from math import sin, cos
        ijkToRas = vtk.vtkMatrix4x4()
        volumeNode.GetIJKToRASMatrix(ijkToRas)
        spacing = volumeNode.GetSpacing()
        center_IJK = [(extent[0] + extent[1]) / 2.0, extent[2], (extent[4] + extent[5]) / 2.0]
        sourcePoints_RAS = numpy.zeros(shape=samplingPoints_shape)
        targetPoints_RAS = numpy.zeros(shape=samplingPoints_shape)
        for k in range(samplingPoints_shape[2]):
            for j in range(samplingPoints_shape[1]):
                for i in range(samplingPoints_shape[0]):
                    samplingPoint_IJK = [gridAxesIJK[0][i], gridAxesIJK[1][j], gridAxesIJK[2][k], 1]
                    sourcePoint_RAS = np.array(ijkToRas.MultiplyPoint(samplingPoint_IJK)[:3])
                    radius = probeRadius - (samplingPoint_IJK[1] - center_IJK[1]) * spacing[1]
                    angleRad = (samplingPoint_IJK[0] - center_IJK[0]) * spacing[0] / probeRadius
                    targetPoint_RAS = np.array([
                        -radius * sin(angleRad),
                        radius * cos(angleRad) - probeRadius,
                        spacing[2] * (samplingPoint_IJK[2] - center_IJK[2])])
                    displacements[k][j][i] = targetPoint_RAS - sourcePoint_RAS

        return gridTransform


#
# DICOMGeAbusPlugin
#

class DICOMGeAbusPlugin:
    """
    This class is the 'hook' for slicer to detect and recognize the plugin
    as a loadable scripted module
    """

    def __init__(self, parent):
        parent.title = "DICOM GE ABUS Import Plugin"
        parent.categories = ["Developer Tools.DICOM Plugins"]
        parent.contributors = ["Andras Lasso (PerkLab)"]
        parent.helpText = """
    Plugin to the DICOM Module to parse and load GE Invenia ABUS images.
    No module interface here, only in the DICOM module.
    """
        parent.acknowledgementText = """
    The file was originally developed by Andras Lasso (PerkLab).
    """

        # don't show this module - it only appears in the DICOM module
        parent.hidden = True

        # Add this extension to the DICOM module's list for discovery when the module
        # is created.  Since this module may be discovered before DICOM itself,
        # create the list if it doesn't already exist.
        try:
            slicer.modules.dicomPlugins
        except AttributeError:
            slicer.modules.dicomPlugins = {}
        slicer.modules.dicomPlugins['DICOMGeAbusPlugin'] = DICOMGeAbusPluginClass
