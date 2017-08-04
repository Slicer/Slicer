import logging
import numpy
import os
import unittest
import urllib
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
from DICOMLib import DICOMUtils

#
# DICOMReaders
#

class DICOMReaders(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    parent.title = "DICOMReaders"
    parent.categories = ["DICOMReaders"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = """
    This module was developed to confirm that different DICOM reading approaches result in the same volumes loaded in Slicer (or that old readers fail but fixed readers succeed).
    """
    parent.acknowledgementText = """This work is supported primarily by the National Institutes of Health, National Cancer Institute, Informatics Technology for Cancer Research (ITCR) program, grant Quantitative Image Informatics for Cancer Research (QIICR) (U24 CA180918, PIs Kikinis and Fedorov). We also acknowledge support of the following grants: Neuroimage Analysis Center (NAC) (P41 EB015902, PI Kikinis) and National Center for Image Guided Therapy (NCIGT) (P41 EB015898, PI Tempany).
    This file was originally developed by Steve Pieper, Isomics, Inc.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['DICOMReaders'] = self.runTest

  def runTest(self):
    tester = DICOMReadersTest()
    tester.runTest()

#
# qDICOMReadersWidget
#

class DICOMReadersWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    # Instantiate and connect widgets ...
    ScriptedLoadableModuleWidget.setup(self)

    self.layout.addStretch(1)

#
# DICOMReadersLogic
#

class DICOMReadersLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    ScriptedLoadableModuleLogic.__init__(self)


class DICOMReadersTest(ScriptedLoadableModuleTest):
  """
  This is the test case
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    self.delayDisplay("Closing the scene", 10)
    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_Part1DICOM()

  def test_Part1DICOM(self):
    """ Test the DICOM loading of sample testing data
    """
    testPass = True
    import os, json
    self.delayDisplay("Starting the DICOM test", 100)

    referenceData = json.JSONDecoder().decode('''[
      { "url": "http://slicer.kitware.com/midas3/download/item/292839/Mouse-MR-example-where-GDCM_fails.zip",
        "fileName": "Mouse-MR-example-where-GDCM_fails.zip",
        "name": "Mouse-MR-example-where-GDCM_fails",
        "seriesUID": "1.3.6.1.4.1.9590.100.1.2.366426457713813178933224342280246227461",
        "expectedFailures": ["GDCM", "Archetype"],
        "voxelValueQuantity": "(110852, DCM, \\"MR signal intensity\\")",
        "voxelValueUnits": "(1, UCUM, \\"no units\\")"
      },
      { "url": "http://slicer.kitware.com/midas3/download/item/294857/deidentifiedMRHead-dcm-one-series.zip",
        "fileName": "deidentifiedMRHead-dcm-one-series.zip",
        "name": "deidentifiedMRHead-dcm-one-series",
        "seriesUID": "1.3.6.1.4.1.5962.99.1.3814087073.479799962.1489872804257.270.0",
        "expectedFailures": [],
        "voxelValueQuantity": "(110852, DCM, \\"MR signal intensity\\")",
        "voxelValueUnits": "(1, UCUM, \\"no units\\")"
      }
    ]''')

    # another dataset that could be added in the future - currently fails for all readers
    # due to invalid format - see https://issues.slicer.org/view.php?id=3569
      #{ "url": "http://slicer.kitware.com/midas3/download/item/293587/RIDER_bug.zip",
        #"fileName": "RIDER_bug.zip",
        #"name": "RIDER_bug",
        #"seriesUID": "1.3.6.1.4.1.9328.50.7.261772317324041365541450388603508531852",
        #"expectedFailures": []
      #}

    loadingResult = {}
    #
    # first, get the data - a zip file of dicom data
    #
    self.delayDisplay("Downloading", 100)
    for dataset in referenceData:
      try:
        filePath = slicer.app.temporaryPath + '/' + dataset['fileName']
        if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
          self.delayDisplay('Requesting download %s from %s...\n' % (dataset['fileName'], dataset['url']), 100)
          urllib.urlretrieve(dataset['url'], filePath)
        self.delayDisplay('Finished with download\n', 100)

        self.delayDisplay("Unzipping", 100)
        dicomFilesDirectory = slicer.app.temporaryPath + dataset['name']
        qt.QDir().mkpath(dicomFilesDirectory)
        slicer.app.applicationLogic().Unzip(filePath, dicomFilesDirectory)

        #
        # insert the data into th database
        #
        self.delayDisplay("Switching to temp database directory", 100)
        originalDatabaseDirectory = DICOMUtils.openTemporaryDatabase('tempDICOMDatabase')

        self.delayDisplay('Importing DICOM', 100)
        mainWindow = slicer.util.mainWindow()
        mainWindow.moduleSelector().selectModule('DICOM')

        indexer = ctk.ctkDICOMIndexer()
        indexer.addDirectory(slicer.dicomDatabase, dicomFilesDirectory, None)
        indexer.waitForImportFinished()

        #
        # selct the series
        #
        detailsPopup = slicer.modules.DICOMWidget.detailsPopup
        detailsPopup.open()
        # load the data by series UID
        detailsPopup.offerLoadables(dataset['seriesUID'],'Series')
        detailsPopup.examineForLoading()
        loadable = detailsPopup.getAllSelectedLoadables().keys()[0]

        #
        # try loading using each of the selected readers, fail
        # on enexpected load issue
        #
        scalarVolumePlugin = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin']()
        readerApproaches = scalarVolumePlugin.readerApproaches()
        basename = loadable.name
        volumesByApproach = {}
        for readerApproach in readerApproaches:
          self.delayDisplay('Loading Selection with approach: %s' % readerApproach, 100)
          loadable.name = basename + "-" + readerApproach
          volumeNode = scalarVolumePlugin.load(loadable,readerApproach)
          if not volumeNode and readerApproach not in dataset['expectedFailures']:
            raise Exception("Expected to be able to read with %s, but couldn't" % readerApproach)
          if volumeNode and readerApproach in dataset['expectedFailures']:
            raise Exception("Expected to NOT be able to read with %s, but could!" % readerApproach)
          if volumeNode:
            volumesByApproach[readerApproach] = volumeNode

            self.delayDisplay('Test quantity and unit')
            if 'voxelValueQuantity' in dataset.keys():
              self.assertEqual(volumeNode.GetVoxelValueQuantity().GetAsPrintableString(), dataset['voxelValueQuantity'])
            if 'voxelValueUnits' in dataset.keys():
              self.assertEqual(volumeNode.GetVoxelValueUnits().GetAsPrintableString(), dataset['voxelValueUnits'])


        #
        # for each approach that loaded as expected, compare the volumes
        # to ensure they match in terms of pixel data and metadata
        #
        failedComparisons = {}
        approachesThatLoaded = volumesByApproach.keys()
        print('approachesThatLoaded %s' % approachesThatLoaded)
        for approachIndex in range(len(approachesThatLoaded)):
          firstApproach = approachesThatLoaded[approachIndex]
          firstVolume = volumesByApproach[firstApproach]
          for secondApproachIndex in range(approachIndex+1,len(approachesThatLoaded)):
            secondApproach = approachesThatLoaded[secondApproachIndex]
            secondVolume = volumesByApproach[secondApproach]
            print('comparing  %s,%s' % (firstApproach, secondApproach))
            comparison = slicer.modules.dicomPlugins['DICOMScalarVolumePlugin'].compareVolumeNodes(firstVolume,secondVolume)
            if comparison != "":
              print('failed: %s', comparison)
              failedComparisons[firstApproach,secondApproach] = comparison

        if len(failedComparisons.keys()) > 0:
          raise Exception("Loaded volumes don't match: %s" % failedComparisons)

        self.delayDisplay('%s Test passed!' % dataset['name'], 200)

      except Exception, e:
        import traceback
        traceback.print_exc()
        self.delayDisplay('%s Test caused exception!\n' % dataset['name'] + str(e), 2000)
        testPass = False

    self.delayDisplay("Restoring original database directory", 200)
    mainWindow.moduleSelector().selectModule('DICOMReaders')

    logging.info(loadingResult)

    return testPass
