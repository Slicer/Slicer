import vtk

import slicer

from DICOMLib import DICOMPlugin
from DICOMLib import DICOMLoadable


#
# This is the plugin for DICOM module
# to import volumes from Enhanced US Volume Storage DICOM files.
#

class DICOMEnhancedUSVolumePluginClass(DICOMPlugin):
    """ 3D ultrasound loader plugin.
    Limitation: ultrasound calibrated regions are not supported (each calibrated region
    would need to be split out to its own volume sequence).
    """

    def __init__(self):
        super().__init__()
        self.loadType = "Enhanced US volume"

        self.tags['sopClassUID'] = "0008,0016"
        self.tags['seriesNumber'] = "0020,0011"
        self.tags['seriesDescription'] = "0008,103E"
        self.tags['instanceNumber'] = "0020,0013"
        self.tags['modality'] = "0008,0060"
        self.tags['photometricInterpretation'] = "0028,0004"

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
            '1.2.840.10008.5.1.4.1.1.6.2',  # Enhanced US Volume Storage
        ]

        # The only sample data set that we received from GE LOGIQE10 (software version R1.5.1).
        # It added all volumes into a single series, even though they were acquired minutes apart.
        # Therefore, instead of loading the volumes into a sequence, we load each as a separate volume.

        loadables = []

        for filePath in files:
            # Quick check of SOP class UID without parsing the file...
            sopClassUID = slicer.dicomDatabase.fileValue(filePath, self.tags['sopClassUID'])
            if not (sopClassUID in supportedSOPClassUIDs):
                # Unsupported class
                continue

            instanceNumber = slicer.dicomDatabase.fileValue(filePath, self.tags['instanceNumber'])
            modality = slicer.dicomDatabase.fileValue(filePath, self.tags['modality'])
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
            else:
                name = f'{name} volume'
            if instanceNumber:
                name = f'{name} [{instanceNumber}]'

            loadable = DICOMLoadable()
            loadable.singleSequence = False  # put each instance in a separate sequence
            loadable.files = [filePath]
            loadable.name = name.strip()  # remove leading and trailing spaces, if any
            loadable.warning = "Loading of this image type is experimental. Please verify image geometry and report any problem is found."
            loadable.tooltip = f"Ultrasound volume"
            loadable.selected = True
            # Confidence is slightly larger than default scalar volume plugin's (0.5)
            # and DICOMVolumeSequencePlugin (0.7)
            # but still leaving room for more specialized plugins.
            loadable.confidence = 0.8
            loadable.grayscale = ('MONOCHROME' in photometricInterpretation)
            loadables.append(loadable)

        return loadables

    def load(self, loadable):
        """Load the selection
        """

        if loadable.grayscale:
            volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScalarVolumeNode", loadable.name)
        else:
            volumeNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLVectorVolumeNode", loadable.name)

        import vtkITK
        if loadable.grayscale:
            reader = vtkITK.vtkITKArchetypeImageSeriesScalarReader()
        else:
            reader = vtkITK.vtkITKArchetypeImageSeriesVectorReaderFile()
        filePath = loadable.files[0]
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

        imageData = reader.GetOutput()
        imageData.SetSpacing(1.0, 1.0, 1.0)
        imageData.SetOrigin(0.0, 0.0, 0.0)
        volumeNode.SetIJKToRASMatrix(ijkToRas)
        volumeNode.SetAndObserveImageData(imageData)

        # show volume
        appLogic = slicer.app.applicationLogic()
        selNode = appLogic.GetSelectionNode()
        selNode.SetActiveVolumeID(volumeNode.GetID())
        appLogic.PropagateVolumeSelection()

        return volumeNode


#
# DICOMEnhancedUSVolumePlugin
#
class DICOMEnhancedUSVolumePlugin:
    """
    This class is the 'hook' for slicer to detect and recognize the plugin
    as a loadable scripted module
    """

    def __init__(self, parent):
        parent.title = "DICOM Enhanced US volume Plugin"
        parent.categories = ["Developer Tools.DICOM Plugins"]
        parent.contributors = ["Andras Lasso (PerkLab)"]
        parent.helpText = """
    Plugin to the DICOM Module to parse and load 3D enhanced US volumes.
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
        slicer.modules.dicomPlugins['DICOMEnhancedUSVolumePlugin'] = DICOMEnhancedUSVolumePluginClass
