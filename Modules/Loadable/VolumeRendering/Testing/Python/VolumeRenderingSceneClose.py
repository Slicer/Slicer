import os
import unittest
import vtk, qt, ctk, slicer

#
# VolumeRenderingSceneClose
#

class VolumeRenderingSceneClose:
  def __init__(self, parent):
    parent.title = "VolumeRenderingSceneClose"
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Nicole Aucoin (BWH)"]
    parent.helpText = """
    This is a scripted self test to check that scene close
    works while in the volume rendering module.
    """
    parent.acknowledgementText = """
    This file was contributed by Nicole Aucoin, BWH, and was partially funded by NIH grant 3P41RR013218-12S1.
"""
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['VolumeRenderingSceneClose'] = self.runTest

  def runTest(self):
    tester = VolumeRenderingSceneCloseTest()
    tester.runTest()

#
# qVolumeRenderingSceneCloseWidget
#

class VolumeRenderingSceneCloseWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    #
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "VolumeRenderingSceneClose Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    #
    # Apply Button
    #
    self.applyButton = qt.QPushButton("Apply")
    self.applyButton.toolTip = "Run the algorithm."
    self.applyButton.enabled = True
    parametersFormLayout.addRow(self.applyButton)

    # connections
    self.applyButton.connect('clicked(bool)', self.onApplyButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onApplyButton(self):
    logic = VolumeRenderingSceneCloseLogic()
    print("Run the algorithm")
    logic.run()

  def onReload(self,moduleName="VolumeRenderingSceneClose"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="VolumeRenderingSceneClose"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(),
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")


#
# VolumeRenderingSceneCloseLogic
#

class VolumeRenderingSceneCloseLogic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def delayDisplay(self,message,msec=1000):
    #
    # logic version of delay display
    #
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def run(self):
    """
    Run the actual algorithm
    """

    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)

    self.delayDisplay('Running the aglorithm')

    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    ctVolume = sampleDataLogic.downloadCTChest()
    self.delayDisplay('Downloaded CT sample data')

    # go to the volume rendering module
    slicer.util.mainWindow().moduleSelector().selectModule('VolumeRendering')
    self.delayDisplay('Volume Rendering module')

    # turn it on
    volumeRenderingWidgetRep = slicer.modules.volumerendering.widgetRepresentation()
    volumeRenderingWidgetRep.setMRMLVolumeNode(ctVolume)
    volumeRenderingNode = slicer.mrmlScene.GetFirstNodeByName('VolumeRendering')
    volumeRenderingNode.SetVisibility(1)
    self.delayDisplay('Volume Rendering')

    # set up a cropping ROI
    volumeRenderingNode.SetCroppingEnabled(1)
    annotationROI = slicer.mrmlScene.GetFirstNodeByName('AnnotationROI')
    annotationROI.SetDisplayVisibility(1)
    self.delayDisplay('Cropping')

    # close the scene
    slicer.mrmlScene.Clear(0)

    return True


class VolumeRenderingSceneCloseTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_VolumeRenderingSceneClose1()

  def test_VolumeRenderingSceneClose1(self):

    self.delayDisplay("Starting the test")

    logic = VolumeRenderingSceneCloseLogic()
    logic.run()

    self.delayDisplay('Test passed!')
