import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# SliceLinkLogic
#

class SliceLinkLogic:
  def __init__(self, parent):
    parent.title = "SliceLinkLogic" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Jim Miller (GE)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module tests the Slice link logic
    """
    parent.acknowledgementText = """
    This file was originally developed by Jim Miller, GE and was partially funded by NIH grant U54EB005149.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['SliceLinkLogic'] = self.runTest

  def runTest(self):
    tester = SliceLinkLogicTest()
    tester.runTest()

#
# qSliceLinkLogicWidget
#

class SliceLinkLogicWidget:
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
    self.reloadButton.name = "SliceLinkLogic Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

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

  def onReload(self,moduleName="SliceLinkLogic"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="SliceLinkLogic"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# SliceLinkLogicLogic
#

class SliceLinkLogicLogic:
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


class SliceLinkLogicTest(unittest.TestCase):
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
    self.test_SliceLinkLogic1()

  def test_SliceLinkLogic1(self):
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
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')
    print('')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = SliceLinkLogicLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )

    eps = 1e-2
    print('eps = ' + str(eps) + '\n')

    # Change to a CompareView
    ln = slicer.util.getNode(pattern='vtkMRMLLayoutNode*')
    ln.SetNumberOfCompareViewRows(3)
    ln.SetNumberOfCompareViewLightboxColumns(4)
    ln.SetViewArrangement(12)
    self.delayDisplay('Compare View')
    print('')

    # Get the slice logic, slice node and slice composite node for the
    # first compare viewer
    logic = slicer.app.layoutManager().sliceWidget('Compare1').sliceLogic()
    compareNode = logic.GetSliceNode()
    compareCNode = logic.GetSliceCompositeNode()

    # Link the viewers
    compareCNode.SetLinkedControl(1)
    self.delayDisplay('Linked the viewers (first Compare View)')

    # Set the data to be same on all viewers
    logic.StartSliceCompositeNodeInteraction(1)  #ForegroundVolumeFlag
    compareCNode.SetForegroundVolumeID(volumeNode.GetID())
    logic.EndSliceCompositeNodeInteraction()
    self.assertTrue( compareCNode.GetForegroundVolumeID() == volumeNode.GetID())
    print('')

    # Check that whether the volume was propagated
    self.delayDisplay('Broadcasted volume selection to all Compare Views')
    compareNode2 = slicer.util.getNode('vtkMRMLSliceNodeCompare2')
    compareCNode2 = slicer.util.getNode('vtkMRMLSliceCompositeNodeCompare2')
    self.assertTrue(compareCNode2.GetForegroundVolumeID() == volumeNode.GetID())
    compareNode3 = slicer.util.getNode('vtkMRMLSliceNodeCompare3')
    compareCNode3 = slicer.util.getNode('vtkMRMLSliceCompositeNodeCompare3')
    self.assertTrue(compareCNode3.GetForegroundVolumeID() == volumeNode.GetID())
    print('')

    # Set the orientation to axial
    logic.StartSliceNodeInteraction(12)  #OrientationFlag & ResetFieldOfViewFlag
    compareNode.SetOrientation('Axial')
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()

    # Reset the field of view
    logic.StartSliceNodeInteraction(8)  #ResetFieldOfViewFlag
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()
    # Note: we validate on fov[1] when resetting the field of view (fov[0] can
    # differ by a few units)
    self.delayDisplay('Broadcasted a reset of the field of view to all Compare Views')
    diff = abs(compareNode2.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print "Field of view of comparison (y) between compare viewers #1 and #2: " + str(diff)
    self.assertTrue(diff < eps)

    diff = abs(compareNode3.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print "Field of view of comparison (y) between compare viewers #1 and #3: " + str(diff)
    self.assertTrue(diff < eps)
    print('')

    # Changed the number of lightboxes
    ln.SetNumberOfCompareViewLightboxColumns(6)
    logic.StartSliceNodeInteraction(8)  #ResetFieldOfViewFlag
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()

    # Note: we validate on fov[1] when resetting the field of view (fov[0] can
    # differ by a few units)
    self.delayDisplay('Changed the number of lightboxes')
    diff = abs(compareNode2.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print "Field of view of comparison (y) between compare viewers #1 and #2: " + str(diff)
    self.assertTrue(diff < eps)

    diff = abs(compareNode3.GetFieldOfView()[1]-compareNode.GetFieldOfView()[1]) / compareNode.GetFieldOfView()[1]
    print "Field of view of comparison between compare viewers #1 and #3: " + str(diff)
    self.assertTrue(diff < eps)
    print('')

    # Pan
    logic.StartSliceNodeInteraction(32) #XYZOriginFlag
    xyz = compareNode.GetXYZOrigin()
    compareNode.SetSliceOrigin(xyz[0] + 50, xyz[1] + 50, xyz[2])
    logic.EndSliceNodeInteraction()

    self.delayDisplay('Broadcasted a pan to all Compare Views')
    diff = abs(compareNode2.GetXYZOrigin()[0]-compareNode.GetXYZOrigin()[0])
    print "Origin comparison (x) between compare viewers #1 and #2: " + str(diff)
    self.assertTrue(diff < eps)

    diff = abs(compareNode3.GetXYZOrigin()[0]-compareNode.GetXYZOrigin()[0])
    print "Origin comparison (x) between compare viewers #1 and #3: " + str(diff)
    self.assertTrue(diff < eps)
    print('')

    # Zoom
    logic.StartSliceNodeInteraction(2) #FieldOfFlag
    fov = compareNode.GetFieldOfView()
    compareNode.SetFieldOfView(fov[0] * 0.5, fov[1] * 0.5, fov[2])
    logic.EndSliceNodeInteraction()
    # Note: we validate on fov[0] when zooming (fov[1] can differ by
    # a few units)
    self.delayDisplay('Broadcasted a zoom to all Compare Views')
    diff = abs(compareNode2.GetFieldOfView()[0]-compareNode.GetFieldOfView()[0]) / compareNode.GetFieldOfView()[0]
    print "Field of view of comparison (x) between compare viewers #1 and #2: " + str(diff)
    self.assertTrue(diff < eps)

    diff = abs(compareNode3.GetFieldOfView()[0]-compareNode.GetFieldOfView()[0]) / compareNode.GetFieldOfView()[0]
    print "Field of view of comparison (x) between compare viewers #1 and #3: " + str(diff)
    self.assertTrue(diff < eps)
    print('')

    # Change the slice
    logic.StartSliceNodeInteraction(1)   #SliceToRAS
    logic.SetSliceOffset(80)
    logic.EndSliceNodeInteraction()
    self.delayDisplay('Broadcasted a change in slice offset to all Compare Views')
    diff = abs(compareNode2.GetSliceOffset()-compareNode.GetSliceOffset())
    print "Slice offset comparison between compare viewers #1 and #2: " + str(diff)
    self.assertTrue(diff < eps)

    diff = abs(compareNode3.GetSliceOffset()-compareNode.GetSliceOffset())
    print "Slice offset comparison between compare viewers #1 and #3: " + str(diff)
    self.assertTrue(diff < eps)
    print('')

    # Change the orientation
    logic.StartSliceNodeInteraction(12)  #OrientationFlag & ResetFieldOfViewFlag
    compareNode.SetOrientation('Sagittal')
    logic.FitSliceToAll()
    compareNode.UpdateMatrices()
    logic.EndSliceNodeInteraction()
    self.delayDisplay('Broadcasted a change in slice orientation to all Compare Views')
    self.assertTrue(compareNode2.GetOrientationString() == compareNode.GetOrientationString())
    self.assertTrue(compareNode3.GetOrientationString() == compareNode.GetOrientationString())
    print('')

    self.delayDisplay('Test passed!')
