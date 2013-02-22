import unittest
import slicer

class qSlicerSampleDataModuleTest(unittest.TestCase):
  def setUp(self):
    mainWindow = slicer.util.mainWindow()
    mainWindow.moduleSelector().selectModule('SampleData')
    module = slicer.modules.sampledata
    self.moduleWidget = module.widgetRepresentation()

  def test_download_volume(self):
    self.moduleWidget.findChild('QPushButton').click()
    scene = slicer.mrmlScene
    self.assertEqual(scene.GetNumberOfNodesByClass('vtkMRMLScalarVolumeNode'), 1)

  def test_download_volume2(self):
    self.moduleWidget.findChild('QPushButton').click()
    scene = slicer.mrmlScene
    self.assertEqual(scene.GetNumberOfNodesByClass('vtkMRMLScalarVolumeNode'), 2)

