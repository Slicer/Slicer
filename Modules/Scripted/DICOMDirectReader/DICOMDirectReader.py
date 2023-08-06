import os
import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import ScriptedLoadableModule


class DICOMDirectReader(ScriptedLoadableModule):
    """A module for loading Revvity Living Image files."""

    def __init__(self, parent):
        """Initialize this class."""
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("DICOMDirectReader")
        self.parent.categories = ["", translate("qSlicerAbstractCoreModule", "Informatics")]  # top level module
        parent.dependencies = ["DICOM"]
        parent.contributors = [""]
        parent.helpText = _("This module allows direct loading of DICOM files without importing into the DICOM database.")
        parent.acknowledgementText = _("This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.")
        # don't show this module - it is only for registering a reader
        parent.hidden = True
        self.parent = parent


class DICOMDirectReaderFileReader:
    """This reader claims DICOM files with higher-than default confidence and loads them directly into the scene.
    """

    def __init__(self, parent):
        self.parent = parent

    def description(self):
        return _('DICOM direct load')

    def fileType(self):
        return 'DICOMDirectLoad'

    def extensions(self):
        return ['DICOM (*.dcm)', 'DICOM (*)']

    def canLoadFileConfidence(self, filePath):
        import pydicom
        if pydicom.misc.is_dicom(filePath):
            # This is a DICOM file, so we return higher confidence than the default 0.5
            # to import DICOM files directly into the scene.
            confidence = 0.6
            if slicer.util.selectedModule() == "DICOM":
                return confidence - 0.05  # rank "DICOM direct load" below "DICOM import"
            return confidence + 0.05  # rank "DICOM direct load" above "DICOM import"
        else:
            return 0.0

    def load(self, properties):
        filePath = properties['fileName']
        dicomFilesDirectory = os.path.dirname(os.path.abspath(filePath))

        loadedNodeIDs = []

        from DICOMLib import DICOMUtils
        with DICOMUtils.TemporaryDICOMDatabase() as db:
            DICOMUtils.importDicom(dicomFilesDirectory, db)
            patientUIDs = db.patients()
            for patientUID in patientUIDs:
                loadedNodeIDs.extend(DICOMUtils.loadPatientByUID(patientUID))

        self.parent.loadedNodes = loadedNodeIDs

        return bool(self.parent.loadedNodes)
