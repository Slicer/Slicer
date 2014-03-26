import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# sceneImport2428
#

class sceneImport2428:
  def __init__(self, parent):
    parent.title = "Scene Import (Issue 2428)" # make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    """
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
    This is a module to support testing of <a>http://www.na-mic.org/Bug/view.php?id=2428</a>
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['sceneImport2428'] = self.runTest

  def runTest(self):
    tester = sceneImport2428Test()
    tester.runTest()

#
# qsceneImport2428Widget
#

class sceneImport2428Widget:
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

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "sceneImport2428 Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.testButton = qt.QPushButton("Test")
    self.testButton.toolTip = "Run the self tests."
    self.layout.addWidget(self.testButton)
    self.testButton.connect('clicked()', self.onTest)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # Collapsible button
    dummyCollapsibleButton = ctk.ctkCollapsibleButton()
    dummyCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(dummyCollapsibleButton)

    # Layout within the dummy collapsible button
    dummyFormLayout = qt.QFormLayout(dummyCollapsibleButton)

    # HelloWorld button
    helloWorldButton = qt.QPushButton("Hello world")
    helloWorldButton.toolTip = "Print 'Hello world' in standard ouput."
    dummyFormLayout.addWidget(helloWorldButton)
    helloWorldButton.connect('clicked(bool)', self.onHelloWorldButtonClicked)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Set local var as instance attribute
    self.helloWorldButton = helloWorldButton

  def onHelloWorldButtonClicked(self):
    print "Hello World !"

  def onReload(self,moduleName="sceneImport2428"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onTest(self,moduleName="sceneImport2428"):
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

  def onReloadAndTest(self):
    self.onReload()
    self.onTest()

#
# sceneImport2428Logic
#

class sceneImport2428Logic:
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass

  def hasImageData(self,volumeNode):
    """This is a dummy logic method that
    returns true if the passed in volume
    node has valid image data
    """
    if not volumeNode:
      print('no volume node')
      return False
    if volumeNode.GetImageData() == None:
      print('no image data')
      return False
    return True


class sceneImport2428Test(unittest.TestCase):
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
    self.test_sceneImport24281()

  def test_sceneImport24281(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests sould exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import SampleData
    sampleDataLogic = SampleData.SampleDataLogic()
    self.delayDisplay("Getting Data")
    head = sampleDataLogic.downloadMRHead()

    #
    # create a label map and set it for editing
    #
    self.delayDisplay("Setting up LabelMap")
    volumesLogic = slicer.modules.volumes.logic()
    headLabel = volumesLogic.CreateAndAddLabelVolume( slicer.mrmlScene, head, head.GetName() + '-label' )
    selectionNode = slicer.app.applicationLogic().GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID( head.GetID() )
    selectionNode.SetReferenceActiveLabelVolumeID( headLabel.GetID() )
    slicer.app.applicationLogic().PropagateVolumeSelection(0)

    #
    # got to the editor and do some drawing
    #
    import EditorLib
    self.delayDisplay("Setting up Editor and drawing")
    editUtil = EditorLib.EditUtil.EditUtil()
    parameterNode = editUtil.getParameterNode()
    lm = slicer.app.layoutManager()
    paintEffectOptions = EditorLib.PaintEffectOptions()
    paintEffectOptions.setMRMLDefaults()
    paintEffectOptions.__del__()

    self.delayDisplay('Paint radius is %s' % parameterNode.GetParameter('PaintEffect,radius'))
    sliceWidget = lm.sliceWidget('Red')
    size = min(sliceWidget.width,sliceWidget.height)
    step = size / 12
    center = size / 2
    parameterNode.SetParameter('PaintEffect,radius', '20')
    paintTool = EditorLib.PaintEffectTool(sliceWidget)
    self.delayDisplay('Paint radius is %s, tool radius is %d' % (parameterNode.GetParameter('PaintEffect,radius'),paintTool.radius))
    for label in xrange(1,5):
      editUtil.setLabel(label)
      pos = center - 2*step + (step * label)
      self.delayDisplay('Painting %d, at  (%d,%d)' % (label,pos,pos),200)
      paintTool.paintAddPoint(pos,pos)
      paintTool.paintApply()
    paintTool.cleanup()
    paintTool = None

    #
    # now build:
    # create a model using the command line module
    # based on the current editor parameters
    # - make a new hierarchy node
    #

    self.delayDisplay( "Building..." )

    parameters = {}
    parameters["InputVolume"] = headLabel.GetID()
    # create models for all labels
    parameters["JointSmoothing"] = True
    parameters["StartLabel"] = -1
    parameters["EndLabel"] = -1
    outHierarchy = slicer.vtkMRMLModelHierarchyNode()
    outHierarchy.SetScene( slicer.mrmlScene )
    outHierarchy.SetName( "sceneImport2428Hierachy" )
    slicer.mrmlScene.AddNode( outHierarchy )
    parameters["ModelSceneFile"] = outHierarchy

    modelMaker = slicer.modules.modelmaker
    self.CLINode = None
    self.CLINode = slicer.cli.run(modelMaker, self.CLINode, parameters, delete_temporary_files=False)
    waitCount = 0
    while self.CLINode.GetStatusString() != 'Completed' and waitCount < 100:
      self.delayDisplay( "Making models... %d" % waitCount )
      waitCount += 1

    self.delayDisplay("Models built")

    success = self.verifyModels()

    success = success and (slicer.mrmlScene.GetNumberOfNodesByClass( "vtkMRMLModelNode" ) > 3)

    self.delayDisplay("Test finished")

    if success:
      self.delayDisplay("Ahh... test passed.")
    else:
      self.delayDisplay("!$!$!#!@#!@!@$%! Test Failed!!")

    self.assertTrue(success)

  def verifyModels(self):
    """Return True if the models have unique polydata and have the
    same polydata as their display nodes have.

# paste this in the slicer console for testing/verifying any scene
def verifyModels():
  nn = getNodes('*ModelNode*')
  for n in nn:
    mpd = nn[n].GetPolyData()
    mdpd = nn[n].GetDisplayNode().GetInputPolyData()
    if mpd != mdpd:
      print(nn[n].GetName())
      print(mpd,mdpd)

verifyModels()
    """

    #
    # now check that all models have the same poly data in the
    # model node as in the display node
    #
    polyDataInScene = []
    fileNamesInScene = []
    success = True
    numModels = slicer.mrmlScene.GetNumberOfNodesByClass( "vtkMRMLModelNode" )
    for n in xrange(numModels):
      modelNode = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelNode" )
      polyDataInScene.append(modelNode.GetPolyData())
      for dn in xrange(modelNode.GetNumberOfDisplayNodes()):
        displayNode = modelNode.GetNthDisplayNode(dn)
        if modelNode.GetPolyData() != displayNode.GetInputPolyData():
          self.delayDisplay("Model %d does not match its display node %d! (name: %s, ids: %s and %s)" % (n,dn,modelNode.GetName(), modelNode.GetID(),displayNode.GetID()))
          success = False
      for sn in xrange(modelNode.GetNumberOfStorageNodes()):
        storageNode = modelNode.GetNthStorageNode(sn)
        fileName = storageNode.GetFileName()
        fileNamesInScene.append(fileName)
        if fileName in fileNamesInScene:
          self.delayDisplay("Model %d has duplicate file name %s! (ids: %s and %s)" % (n,fileName,modelNode.GetID(),storageNode.GetID()))
          success = False


    #
    # now check that each model has a unique polydata
    #
    for n in xrange(numModels):
      modelNode = slicer.mrmlScene.GetNthNodeByClass( n, "vtkMRMLModelNode" )
      if polyDataInScene.count(modelNode.GetPolyData()) > 1:
        self.delayDisplay("Polydata for Model is duplicated! (id: %s and %s)" % (n,modelNode.GetID()))
        success = False

    return success
