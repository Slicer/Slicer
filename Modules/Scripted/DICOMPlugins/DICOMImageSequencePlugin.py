import logging

import pydicom as dicom
import vtk

import slicer

from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable


#
# This is the plugin to handle translation of DICOM objects
# that can be represented as multivolume objects
# from DICOM files into MRML nodes.  It follows the DICOM module's
# plugin architecture.
#

class DICOMImageSequencePluginClass(DICOMPlugin):
    """ 2D image sequence loader plugin.
    It supports X-ray angiography and ultrasound images.
    The main difference compared to plain scalar volume plugin is that it
    loads frames as a single-slice-volume sequence (and not as a 3D volume),
    it accepts color images, and handles multiple instances within a series
    (e.g., multiple independent acquisitions and synchronized biplane acquisitions).
    Limitation: ultrasound calibrated regions are not supported (each calibrated region
    would need to be split out to its own volume sequence).
    """

    def __init__(self):
        super().__init__()
        self.loadType = "Image sequence"

        self.tags['sopClassUID'] = "0008,0016"
        self.tags['seriesNumber'] = "0020,0011"
        self.tags['seriesDescription'] = "0008,103E"
        self.tags['instanceNumber'] = "0020,0013"
        self.tags['triggerTime'] = "0018,1060"
        self.tags['modality'] = "0008,0060"
        self.tags['photometricInterpretation'] = "0028,0004"
        self.tags['orientation'] = "0020,0037"

        self.detailedLogging = False

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

        self.detailedLogging = slicer.util.settingsValue('DICOM/detailedLogging', False, converter=slicer.util.toBool)

        supportedSOPClassUIDs = [
            '1.2.840.10008.5.1.4.1.1.12.1',  # X-Ray Angiographic Image Storage
            '1.2.840.10008.5.1.4.1.1.12.2',  # X-Ray Fluoroscopy Image Storage
            '1.2.840.10008.5.1.4.1.1.3.1',  # Ultrasound Multiframe Image Storage
            '1.2.840.10008.5.1.4.1.1.6.1',  # Ultrasound Image Storage
            '1.2.840.10008.5.1.4.1.1.7',  # Secondary Capture Image Storage (only accepted for modalities that typically acquire 2D image sequences)
            '1.2.840.10008.5.1.4.1.1.4',  # MR Image Storage (will be only accepted if cine-MRI)
        ]

        # Modalities that typically acquire 2D image sequences:
        suppportedSecondaryCaptureModalities = ['US', 'XA', 'RF', 'ES']

        # Each instance will be a loadable, that will result in one sequence browser node
        # and usually one sequence (except simultaneous biplane acquisition, which will
        # result in two sequences).
        # Each pedal press on the XA/RF acquisition device creates a new instance number,
        # but if the device has two imaging planes (biplane) then two sequences
        # will be acquired, which have the same instance number. These two sequences
        # are synchronized in time, therefore they have to be assigned to the same
        # browser node.
        instanceNumberToLoadableIndex = {}

        loadables = []

        canBeCineMri = True
        cineMriTriggerTimes = set()
        cineMriImageOrientations = set()
        cineMriInstanceNumberToFilenameIndex = {}

        for filePath in files:
            # Quick check of SOP class UID without parsing the file...
            try:
                sopClassUID = slicer.dicomDatabase.fileValue(filePath, self.tags['sopClassUID'])
                if not (sopClassUID in supportedSOPClassUIDs):
                    # Unsupported class
                    continue

                # Only accept MRI if it looks like cine-MRI
                if sopClassUID != '1.2.840.10008.5.1.4.1.1.4':  # MR Image Storage (will be only accepted if cine-MRI)
                    canBeCineMri = False
                if not canBeCineMri and sopClassUID == '1.2.840.10008.5.1.4.1.1.4':  # MR Image Storage
                    continue

            except Exception as e:
                # Quick check could not be completed (probably Slicer DICOM database is not initialized).
                # No problem, we'll try to parse the file and check the SOP class UID then.
                pass

            instanceNumber = slicer.dicomDatabase.fileValue(filePath, self.tags['instanceNumber'])
            if canBeCineMri and sopClassUID == '1.2.840.10008.5.1.4.1.1.4':  # MR Image Storage
                if not instanceNumber:
                    # no instance number, probably not cine-MRI
                    canBeCineMri = False
                    if self.detailedLogging:
                        logging.debug("No instance number attribute found, the series will not be considered as a cine MRI")
                    continue
                cineMriInstanceNumberToFilenameIndex[int(instanceNumber)] = filePath
                cineMriTriggerTimes.add(slicer.dicomDatabase.fileValue(filePath, self.tags['triggerTime']))
                cineMriImageOrientations.add(slicer.dicomDatabase.fileValue(filePath, self.tags['orientation']))

            else:
                modality = slicer.dicomDatabase.fileValue(filePath, self.tags['modality'])
                if sopClassUID == '1.2.840.10008.5.1.4.1.1.7':  # Secondary Capture Image Storage
                    if modality not in suppportedSecondaryCaptureModalities:
                        # practice of dumping secondary capture images into the same series
                        # is only prevalent in US and XA/RF modalities
                        continue

                if not (instanceNumber in instanceNumberToLoadableIndex.keys()):
                    # new instance number
                    seriesNumber = slicer.dicomDatabase.fileValue(filePath, self.tags['seriesNumber'])
                    seriesDescription = slicer.dicomDatabase.fileValue(filePath, self.tags['seriesDescription'])
                    photometricInterpretation = slicer.dicomDatabase.fileValue(filePath, self.tags['photometricInterpretation'])
                    name = ''
                    if seriesNumber:
                        name = f'{seriesNumber}:'
                    if modality:
                        name = f'{name} {modality}'
                    if seriesDescription:
                        name = f'{name} {seriesDescription}'
                    if instanceNumber:
                        name = f'{name} [{instanceNumber}]'

                    loadable = DICOMLoadable()
                    loadable.singleSequence = False  # put each instance in a separate sequence
                    loadable.files = [filePath]
                    loadable.name = name.strip()  # remove leading and trailing spaces, if any
                    loadable.warning = "Image spacing may need to be calibrated for accurate size measurements."
                    loadable.tooltip = f"{modality} image sequence"
                    loadable.selected = True
                    # Confidence is slightly larger than default scalar volume plugin's (0.5)
                    # but still leaving room for more specialized plugins.
                    loadable.confidence = 0.7
                    loadable.grayscale = ('MONOCHROME' in photometricInterpretation)

                    # Add to loadables list
                    loadables.append(loadable)
                    instanceNumberToLoadableIndex[instanceNumber] = len(loadables) - 1
                else:
                    # existing instance number, add this file
                    loadableIndex = instanceNumberToLoadableIndex[instanceNumber]
                    loadables[loadableIndex].files.append(filePath)
                    loadable.tooltip = f"{modality} image sequence ({len(loadables[loadableIndex].files)} planes)"

        if canBeCineMri and len(cineMriInstanceNumberToFilenameIndex) > 1:
            # Get description from first
            ds = dicom.read_file(cineMriInstanceNumberToFilenameIndex[next(iter(cineMriInstanceNumberToFilenameIndex))], stop_before_pixels=True)
            name = ''
            if hasattr(ds, 'SeriesNumber') and ds.SeriesNumber:
                name = f'{ds.SeriesNumber}:'
            if hasattr(ds, 'Modality') and ds.Modality:
                name = f'{name} {ds.Modality}'
            if hasattr(ds, 'SeriesDescription') and ds.SeriesDescription:
                name = f'{name} {ds.SeriesDescription}'

            loadable = DICOMLoadable()
            loadable.singleSequence = True  # put all instances in a single sequence
            loadable.instanceNumbers = sorted(cineMriInstanceNumberToFilenameIndex)
            loadable.files = [cineMriInstanceNumberToFilenameIndex[instanceNumber] for instanceNumber in loadable.instanceNumbers]
            loadable.name = name.strip()  # remove leading and trailing spaces, if any
            loadable.tooltip = f"{ds.Modality} image sequence"
            loadable.selected = True
            if len(cineMriTriggerTimes) > 3:
                if self.detailedLogging:
                    logging.debug("Several different trigger times found (" + repr(cineMriTriggerTimes) + ") - assuming this series is a cine MRI")
                # This is likely a cardiac cine acquisition.
                if len(cineMriImageOrientations) > 1:
                    if self.detailedLogging:
                        logging.debug("Several different image orientations found (" + repr(cineMriImageOrientations) + ") - assuming this series is a rotational cine MRI")
                    # Multivolume importer sets confidence=0.9-1.0, so we need to set a bit higher confidence to be selected by default
                    loadable.confidence = 1.05
                else:
                    if self.detailedLogging:
                        logging.debug("All image orientations are the same (" + repr(cineMriImageOrientations) + ") - probably the MultiVolume plugin should load this")
                    # Multivolume importer sets confidence=0.9-1.0, so we need to set a bit lower confidence to allow multivolume selected by default
                    loadable.confidence = 0.85
            else:
                # This may be a 3D acquisition,so set lower confidence than scalar volume's default (0.5)
                if self.detailedLogging:
                    logging.debug("Only one or few different trigger times found (" + repr(cineMriTriggerTimes) + ") - assuming this series is not a cine MRI")
                loadable.confidence = 0.4
            loadable.grayscale = ('MONOCHROME' in ds.PhotometricInterpretation)

            # Add to loadables list
            loadables.append(loadable)

        return loadables

    def loadImageData(self, filePath, grayscale, volumeNode):
        import vtkITK
        if grayscale:
            reader = vtkITK.vtkITKArchetypeImageSeriesScalarReader()
        else:
            reader = vtkITK.vtkITKArchetypeImageSeriesVectorReaderFile()
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
        if reader.GetErrorCode() != vtk.vtkErrorCode.NoError:
            errorString = vtk.vtkErrorCode.GetStringFromErrorCode(reader.GetErrorCode())
            raise ValueError(
                f"Could not read image {loadable.name} from file {filePath}. Error is: {errorString}")

        rasToIjk = reader.GetRasToIjkMatrix()
        ijkToRas = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Invert(rasToIjk, ijkToRas)
        return reader.GetOutput(), ijkToRas

    def addSequenceBrowserNode(self, name, outputSequenceNodes, playbackRateFps, loadable):
        # Add a browser node and show the volume in the slice viewer for user convenience
        outputSequenceBrowserNode = slicer.vtkMRMLSequenceBrowserNode()
        outputSequenceBrowserNode.SetName(slicer.mrmlScene.GenerateUniqueName(name + ' browser'))
        outputSequenceBrowserNode.SetPlaybackRateFps(playbackRateFps)
        slicer.mrmlScene.AddNode(outputSequenceBrowserNode)

        # Add all sequences to the sequence browser
        first = True
        for outputSequenceNode in outputSequenceNodes:
            outputSequenceBrowserNode.AddSynchronizedSequenceNode(outputSequenceNode)
            proxyVolumeNode = outputSequenceBrowserNode.GetProxyNode(outputSequenceNode)
            # create Subject hierarchy nodes for the loaded series
            self.addSeriesInSubjectHierarchy(loadable, proxyVolumeNode)

            if first:
                first = False
                # Automatically select the volume to display
                appLogic = slicer.app.applicationLogic()
                selNode = appLogic.GetSelectionNode()
                selNode.SetReferenceActiveVolumeID(proxyVolumeNode.GetID())
                appLogic.PropagateVolumeSelection()
                appLogic.FitSliceToAll()
                slicer.modules.sequences.setToolBarActiveBrowserNode(outputSequenceBrowserNode)

        # Show sequence browser toolbar
        slicer.modules.sequences.showSequenceBrowser(outputSequenceBrowserNode)

    def addSequenceFromImageData(self, imageData, tempFrameVolume, filePath, name, singleFileInLoadable):

        # Rotate 180deg, otherwise the image would appear upside down
        ijkToRas = vtk.vtkMatrix4x4()
        ijkToRas.SetElement(0, 0, -1.0)
        ijkToRas.SetElement(1, 1, -1.0)
        tempFrameVolume.SetIJKToRASMatrix(ijkToRas)
        # z axis is time
        [spacingX, spacingY, frameTimeMsec] = imageData.GetSpacing()
        imageData.SetSpacing(1.0, 1.0, 1.0)
        tempFrameVolume.SetSpacing(spacingX, spacingY, 1.0)

        # Create new sequence
        outputSequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode")

        # Get sequence name
        if singleFileInLoadable:
            outputSequenceNode.SetName(name)
        else:
            ds = dicom.read_file(filePath, stop_before_pixels=True)
            if hasattr(ds, 'PositionerPrimaryAngle') and hasattr(ds, 'PositionerSecondaryAngle'):
                outputSequenceNode.SetName(f'{name} ({ds.PositionerPrimaryAngle}/{ds.PositionerSecondaryAngle})')
            else:
                outputSequenceNode.SetName(name)

        if frameTimeMsec == 1.0:
            # frame time is not found, set it to 1.0fps
            frameTime = 1
            outputSequenceNode.SetIndexName("frame")
            outputSequenceNode.SetIndexUnit("")
            playbackRateFps = 10
        else:
            # frame time is set, use it
            frameTime = frameTimeMsec * 0.001
            outputSequenceNode.SetIndexName("time")
            outputSequenceNode.SetIndexUnit("s")
            playbackRateFps = 1.0 / frameTime

        # Add frames to the sequence
        numberOfFrames = imageData.GetDimensions()[2]
        extent = imageData.GetExtent()
        numberOfFrames = extent[5] - extent[4] + 1
        for frame in range(numberOfFrames):
            # get current frame from multiframe
            crop = vtk.vtkImageClip()
            crop.SetInputData(imageData)
            crop.SetOutputWholeExtent(extent[0], extent[1], extent[2], extent[3], extent[4] + frame, extent[4] + frame)
            crop.ClipDataOn()
            crop.Update()
            croppedOutput = crop.GetOutput()
            croppedOutput.SetExtent(extent[0], extent[1], extent[2], extent[3], 0, 0)
            croppedOutput.SetOrigin(0.0, 0.0, 0.0)
            tempFrameVolume.SetAndObserveImageData(croppedOutput)
            # get timestamp
            if type(frameTime) == int:
                timeStampSec = str(frame * frameTime)
            else:
                timeStampSec = f"{frame * frameTime:.3f}"
            outputSequenceNode.SetDataNodeAtValue(tempFrameVolume, timeStampSec)

        # Create storage node that allows saving node as nrrd
        outputSequenceStorageNode = slicer.vtkMRMLVolumeSequenceStorageNode()
        slicer.mrmlScene.AddNode(outputSequenceStorageNode)
        outputSequenceNode.SetAndObserveStorageNodeID(outputSequenceStorageNode.GetID())

        return outputSequenceNode, playbackRateFps

    def load(self, loadable):
        """Load the selection
        """

        outputSequenceNodes = []

        if loadable.singleSequence:
            outputSequenceNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLSequenceNode")
            outputSequenceNode.SetName(loadable.name)
            outputSequenceNode.SetIndexName("instance number")
            outputSequenceNode.SetIndexUnit("")
            playbackRateFps = 10
            outputSequenceNodes.append(outputSequenceNode)

        # Create a temporary volume node that will be used to insert volume nodes in the sequence
        if loadable.grayscale:
            tempFrameVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode")
        else:
            tempFrameVolume = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode")

        for fileIndex, filePath in enumerate(loadable.files):
            imageData, ijkToRas = self.loadImageData(filePath, loadable.grayscale, tempFrameVolume)
            if loadable.singleSequence:
                # each file is a frame (cine-MRI)
                imageData.SetSpacing(1.0, 1.0, 1.0)
                imageData.SetOrigin(0.0, 0.0, 0.0)
                tempFrameVolume.SetIJKToRASMatrix(ijkToRas)
                tempFrameVolume.SetAndObserveImageData(imageData)
                instanceNumber = loadable.instanceNumbers[fileIndex]
                # Save DICOM SOP instance UID into the sequence so DICOM metadata can be retrieved later if needed
                tempFrameVolume.SetAttribute('DICOM.instanceUIDs', slicer.dicomDatabase.instanceForFile(filePath))
                # Save trigger time, because it may be needed for 4D cine-MRI volume reconstruction
                triggerTime = slicer.dicomDatabase.fileValue(filePath, self.tags['triggerTime'])
                if triggerTime:
                    tempFrameVolume.SetAttribute('DICOM.triggerTime', triggerTime)
                outputSequenceNode.SetDataNodeAtValue(tempFrameVolume, str(instanceNumber))
            else:
                # each file is a new sequence
                outputSequenceNode, playbackRateFps = self.addSequenceFromImageData(
                    imageData, tempFrameVolume, filePath, loadable.name, (len(loadable.files) == 1))
                outputSequenceNodes.append(outputSequenceNode)

        # Delete temporary volume node
        slicer.mrmlScene.RemoveNode(tempFrameVolume)

        if not hasattr(loadable, 'createBrowserNode') or loadable.createBrowserNode:
            self.addSequenceBrowserNode(loadable.name, outputSequenceNodes, playbackRateFps, loadable)

        # Return the last loaded sequence node (that is the one currently displayed in slice views)
        return outputSequenceNodes[-1]


#
# DICOMImageSequencePlugin
#

class DICOMImageSequencePlugin:
    """
    This class is the 'hook' for slicer to detect and recognize the plugin
    as a loadable scripted module
    """

    def __init__(self, parent):
        parent.title = "DICOM Image Sequence Import Plugin"
        parent.categories = ["Developer Tools.DICOM Plugins"]
        parent.contributors = ["Andras Lasso (PerkLab)"]
        parent.helpText = """
    Plugin to the DICOM Module to parse and load 2D image sequences.
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
        slicer.modules.dicomPlugins['DICOMImageSequencePlugin'] = DICOMImageSequencePluginClass
