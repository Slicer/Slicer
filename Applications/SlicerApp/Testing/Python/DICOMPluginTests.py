import os
import shutil
import tempfile
import urllib.request

import slicer
from slicer.ScriptedLoadableModule import *
from DICOMLib import DICOMUtils


#
# DICOMPluginTests
#


class DICOMPluginTests(ScriptedLoadableModule):
    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        parent.title = "DICOMPluginTests"
        parent.categories = ["DICOMPluginTests"]
        parent.dependencies = []
        parent.contributors = ["Andrey Fedorov (SPL, BWH)"]
        parent.helpText = """
    Integration tests for DICOM plugins added in Slicer core: DICOMSegmentationPlugin,
    DICOMTID1500Plugin, and DICOMM3DPlugin. Each test downloads a small (~5-32 KB) reference
    DICOM file from the NCI Imaging Data Commons (IDC, https://portal.imaging.datacommons.cancer.gov/)
    public GCS bucket, imports it into a temporary DICOM database, and verifies that the plugin
    correctly examines and loads the object into the MRML scene.
    """
        parent.acknowledgementText = (
            "Test data sourced from the NCI Imaging Data Commons (IDC). "
            "SEG and SR samples created by the QIICR project (NIH U24 CA180918). "
            "M3D sample from the Prostate MRI US Biopsy collection."
        )


#
# DICOMPluginTestsWidget
#


class DICOMPluginTestsWidget(ScriptedLoadableModuleWidget):
    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)
        self.layout.addStretch(1)


#
# DICOMPluginTestsTest
#


class DICOMPluginTestsTest(ScriptedLoadableModuleTest):
    """Integration tests for DICOMSegmentationPlugin, DICOMTID1500Plugin, and DICOMM3DPlugin."""

    def setUp(self):
        self.delayDisplay("Closing the scene")
        slicer.mrmlScene.Clear(0)

    def runTest(self):
        self.setUp()
        self.test_DICOMSegmentationPlugin()
        self.setUp()
        self.test_DICOMTID1500Plugin()
        self.setUp()
        self.test_DICOMM3DPlugin()

    # -------------------------------------------------------------------------
    # Helpers
    # -------------------------------------------------------------------------

    def _downloadDICOMFile(self, url, destDir):
        """Download a single DICOM file via HTTPS into destDir. Returns the local path."""
        filename = url.split("/")[-1]
        destPath = os.path.join(destDir, filename)
        self.delayDisplay(f"Downloading {filename} ...")
        urllib.request.urlretrieve(url, destPath)
        return destPath

    # -------------------------------------------------------------------------
    # Test: DICOMSegmentationPlugin
    # -------------------------------------------------------------------------

    def test_DICOMSegmentationPlugin(self):
        """Test loading a DICOM SEG object via DICOMSegmentationPlugin.

        Test data: ReMIND collection, patient ReMIND-004, tumor segmentation
        (QIICR/dcmqi-generated, Manufacturer: QIICR).
        SeriesInstanceUID: 1.3.6.1.4.1.14519.5.2.1.27661008822762431370973483522020876264
        License: CC BY 4.0

        IDC viewer:
          https://viewer.imaging.datacommons.cancer.gov/v3/viewer/?StudyInstanceUIDs=
          1.3.6.1.4.1.14519.5.2.1.21953915370182127679153341817124092346
          &initialSeriesInstanceUID=
          1.3.6.1.4.1.14519.5.2.1.27661008822762431370973483522020876264

        Acknowledgment:
          Juvekar, P., et al. (2023). ReMIND: The Brain Resection Multimodal Imaging Database.
          The Cancer Imaging Archive. https://doi.org/10.7937/3RAG-D070
        """
        self.delayDisplay("Starting test_DICOMSegmentationPlugin")

        SEG_URL = (
            "https://storage.googleapis.com/idc-open-data/"
            "8676c8bb-3bdd-4106-be09-97223b5126e7/"
            "6936cea9-f0ac-49bd-91c4-86904f5c3650.dcm"
        )
        SEG_SERIES_UID = "1.3.6.1.4.1.14519.5.2.1.27661008822762431370973483522020876264"

        tmpDir = tempfile.mkdtemp()
        try:
            self._downloadDICOMFile(SEG_URL, tmpDir)

            with DICOMUtils.TemporaryDICOMDatabase() as db:
                self.assertTrue(db.isOpen, "Temporary DICOM database failed to open")

                DICOMUtils.importDicom(tmpDir, db)

                files = slicer.dicomDatabase.filesForSeries(SEG_SERIES_UID)
                self.assertGreater(len(files), 0, "SEG series was not indexed into the DICOM database")

                plugin = slicer.modules.dicomPlugins["DICOMSegmentationPlugin"]()
                loadables = plugin.examineFiles(files)
                self.assertGreater(len(loadables), 0,
                                   "DICOMSegmentationPlugin.examineFiles() returned no loadables")

                self.assertTrue(plugin.load(loadables[0]),
                                "DICOMSegmentationPlugin.load() returned False")

                segNodes = slicer.util.getNodesByClass("vtkMRMLSegmentationNode")
                self.assertEqual(len(segNodes), 1,
                                 f"Expected 1 vtkMRMLSegmentationNode, got {len(segNodes)}")
                self.assertGreater(segNodes[0].GetSegmentation().GetNumberOfSegments(), 0,
                                   "Loaded segmentation node contains no segments")

        finally:
            shutil.rmtree(tmpDir)

        self.delayDisplay("test_DICOMSegmentationPlugin passed!")

    # -------------------------------------------------------------------------
    # Test: DICOMTID1500Plugin
    # -------------------------------------------------------------------------

    def test_DICOMTID1500Plugin(self):
        """Test loading a DICOM SR TID1500 structured report via DICOMTID1500Plugin.

        Test data: QIN-PROSTATE-Repeatability collection, patient PCAMPMRI-00001,
        DCE Subtraction Measurements report (QIICR/dcmqi-generated, Manufacturer: QIICR).
        SeriesInstanceUID: 1.2.276.0.7230010.3.1.3.3166326398.17020.1513205119.297
        SOPClassUID: 1.2.840.10008.5.1.4.1.1.88.22 (Comprehensive SR Storage)
        License: CC BY 4.0

        IDC viewer:
          https://viewer.imaging.datacommons.cancer.gov/v3/viewer/?StudyInstanceUIDs=
          1.3.6.1.4.1.14519.5.2.1.3671.4754.288848219213026850354055725664
          &initialSeriesInstanceUID=
          1.2.276.0.7230010.3.1.3.3166326398.17020.1513205119.297

        Acknowledgment:
          Fedorov, A., Tempany, C., & Fennessy, F. (2019). Data From QIN-PROSTATE-Repeatability
          (Version 2). The Cancer Imaging Archive. https://doi.org/10.7937/K9/TCIA.2018.MR1CKGND
        """
        self.delayDisplay("Starting test_DICOMTID1500Plugin")

        SR_URL = (
            "https://storage.googleapis.com/idc-open-data/"
            "72555341-016b-4483-a4e9-928e03a12b17/"
            "6cfb33f2-65a7-413f-9e01-d06aaf38f2ef.dcm"
        )
        SR_SERIES_UID = "1.2.276.0.7230010.3.1.3.3166326398.17020.1513205119.297"

        tmpDir = tempfile.mkdtemp()
        try:
            self._downloadDICOMFile(SR_URL, tmpDir)

            with DICOMUtils.TemporaryDICOMDatabase() as db:
                self.assertTrue(db.isOpen, "Temporary DICOM database failed to open")

                DICOMUtils.importDicom(tmpDir, db)

                files = slicer.dicomDatabase.filesForSeries(SR_SERIES_UID)
                self.assertGreater(len(files), 0, "SR series was not indexed into the DICOM database")

                plugin = slicer.modules.dicomPlugins["DICOMTID1500Plugin"]()
                loadables = plugin.examineFiles(files)
                self.assertGreater(len(loadables), 0,
                                   "DICOMTID1500Plugin.examineFiles() returned no loadables")

                self.assertTrue(plugin.load(loadables[0]),
                                "DICOMTID1500Plugin.load() returned False")

                tableNodes = slicer.util.getNodesByClass("vtkMRMLTableNode")
                self.assertGreater(len(tableNodes), 0,
                                   "Expected at least one vtkMRMLTableNode after loading TID1500 SR")

        finally:
            shutil.rmtree(tmpDir)

        self.delayDisplay("test_DICOMTID1500Plugin passed!")

    # -------------------------------------------------------------------------
    # Test: DICOMM3DPlugin
    # -------------------------------------------------------------------------

    def test_DICOMM3DPlugin(self):
        """Test loading a DICOM M3D object (encapsulated STL) via DICOMM3DPlugin.

        Test data: Prostate-MRI-US-Biopsy collection, patient Prostate-MRI-US-Biopsy-0293,
        STL surface of lesion 2 (DICOM annotations by Ciausu, Clunie, Fedorov).
        SeriesInstanceUID: 1.3.6.1.4.1.5962.99.1.1972223737.1083348708.1694189371129.3.0
        SOPClassUID: 1.2.840.10008.5.1.4.1.1.104.3
        License: CC BY 4.0

        IDC viewer:
          https://viewer.imaging.datacommons.cancer.gov/v3/viewer/?StudyInstanceUIDs=
          1.3.6.1.4.1.14519.5.2.1.298134725212789995817164419725804739814
          &initialSeriesInstanceUID=
          1.3.6.1.4.1.5962.99.1.1972223737.1083348708.1694189371129.3.0

        Acknowledgment:
          Ciausu, C., Clunie, D., & Fedorov, A. (2023). DICOM converted annotations for the
          Prostate-MRI-US-Biopsy collection [Data set]. Zenodo.
          https://doi.org/10.5281/ZENODO.10069910
        """
        self.delayDisplay("Starting test_DICOMM3DPlugin")

        M3D_URL = (
            "https://storage.googleapis.com/idc-open-data/"
            "e5db2185-8d0a-4e35-aa63-231ad7aa6e1b/"
            "c9a91b1e-0bb4-4f35-98b9-c55aa6217e22.dcm"
        )
        M3D_SERIES_UID = "1.3.6.1.4.1.5962.99.1.1972223737.1083348708.1694189371129.3.0"

        tmpDir = tempfile.mkdtemp()
        try:
            self._downloadDICOMFile(M3D_URL, tmpDir)

            with DICOMUtils.TemporaryDICOMDatabase() as db:
                self.assertTrue(db.isOpen, "Temporary DICOM database failed to open")

                DICOMUtils.importDicom(tmpDir, db)

                files = slicer.dicomDatabase.filesForSeries(M3D_SERIES_UID)
                self.assertGreater(len(files), 0, "M3D series was not indexed into the DICOM database")

                plugin = slicer.modules.dicomPlugins["DICOMM3DPlugin"]()
                loadables = plugin.examineFiles(files)
                self.assertGreater(len(loadables), 0,
                                   "DICOMM3DPlugin.examineFiles() returned no loadables")

                self.assertTrue(plugin.load(loadables[0]),
                                "DICOMM3DPlugin.load() returned False")

                segNodes = slicer.util.getNodesByClass("vtkMRMLSegmentationNode")
                self.assertEqual(len(segNodes), 1,
                                 f"Expected 1 vtkMRMLSegmentationNode, got {len(segNodes)}")

        finally:
            shutil.rmtree(tmpDir)

        self.delayDisplay("test_DICOMM3DPlugin passed!")
