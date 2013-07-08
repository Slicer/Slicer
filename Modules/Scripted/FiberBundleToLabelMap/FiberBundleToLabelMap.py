import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# FiberBundleToLabelMap
#

class FiberBundleToLabelMap:
  def __init__(self, parent):
    import string
    parent.title = "Fiber Bundle to Label Map"
    parent.categories = ["Diffusion.Tractography"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = string.Template("""
See <a href=\"$a/Documentation/$b.$c/Modules/FiberBuntdeToLabelMap\">the documentation</a> for more information.

This module converts Fiber Bundles to Label Map Volumes by filling pixels in the label volume where fiber vertex points are located.
Note that if the pixel spacing of the target label map is finer than the spacing of the fiber bundle vertices it is possible to have gaps.
Also note that a label pixel is discretely turned on even if many fibers pass through the same pixel or if only one fiber passes through a corner.
For these reasons please use care when interpreting the results.
    """).substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
    This file was originally developed by Steve Pieper, Isomics, Inc.  and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['FiberBundleToLabelMap'] = self.runTest

  def runTest(self):
    tester = FiberBundleToLabelMapTest()
    tester.runTest()

#
# qFiberBundleToLabelMapWidget
#

class FiberBundleToLabelMapWidget:
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

    # Collapsible button
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the parameters collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    # fiber
    self.fiberSelector = slicer.qMRMLNodeComboBox(parametersCollapsibleButton)
    self.fiberSelector.nodeTypes = ( ("vtkMRMLFiberBundleNode"), "" )
    self.fiberSelector.selectNodeUponCreation = False
    self.fiberSelector.addEnabled = False
    self.fiberSelector.removeEnabled = False
    self.fiberSelector.noneEnabled = True
    self.fiberSelector.showHidden = False
    self.fiberSelector.showChildNodeTypes = False
    self.fiberSelector.setMRMLScene( slicer.mrmlScene )
    self.fiberSelector.setToolTip( "Pick the fiber bundle to be converted." )
    parametersFormLayout.addRow("Fiber Bundle", self.fiberSelector)

    # label map
    self.labelSelector = slicer.qMRMLNodeComboBox(parametersCollapsibleButton)
    self.labelSelector.nodeTypes = ( ("vtkMRMLScalarVolumeNode"), "" )
    self.labelSelector.addAttribute( "vtkMRMLScalarVolumeNode", "LabelMap", 1 )
    self.labelSelector.selectNodeUponCreation = False
    self.labelSelector.addEnabled = False
    self.labelSelector.removeEnabled = False
    self.labelSelector.noneEnabled = True
    self.labelSelector.showHidden = False
    self.labelSelector.showChildNodeTypes = False
    self.labelSelector.setMRMLScene( slicer.mrmlScene )
    self.labelSelector.setToolTip( "Pick the target label volume.  Must already exists in order to define sampling grid.  If needed, create one in the Editor module based on template volume." )
    parametersFormLayout.addRow("Target LabelMap", self.labelSelector)

    # label value
    self.labelValue = qt.QSpinBox(parametersCollapsibleButton)
    self.labelValue.setToolTip( "The numerical value for the rasterized fiber label." )
    self.labelValue.setValue(1)
    parametersFormLayout.addRow("Label Value", self.labelValue)

    # apply
    self.applyButton = qt.QPushButton(parametersCollapsibleButton)
    self.applyButton.text = "Apply"
    parametersFormLayout.addWidget(self.applyButton)

    self.applyButton.connect('clicked()', self.onApply)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onApply(self):
    fiberNode = self.fiberSelector.currentNode()
    labelNode = self.labelSelector.currentNode()
    if not fiberNode and not labelNode:
      qt.QMessageBox.critical(slicer.util.mainWindow(), 'FiberBundleToLabelMap', "Must select fiber bundle and label map")

    logic = FiberBundleToLabelMapLogic()
    logic.rasterizeFibers(fiberNode, labelNode, self.labelValue.value)

#
# FiberBundleToLabelMapLogic
#

class FiberBundleToLabelMapLogic:
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

  def rasterizeFibers(self,fiberNode,labelNode,labelValue=1):
    """Trace through the given fiber bundles and
    set the corresponding pixels in the given labelNode volume"""
    print('rasterizing...')
    rasToIJK = vtk.vtkMatrix4x4()
    labelNode.GetRASToIJKMatrix(rasToIJK)
    labelArray = slicer.util.array(labelNode.GetID())
    polyData = fiberNode.GetPolyData()
    pointCount = polyData.GetNumberOfPoints()
    points= polyData.GetPoints()
    for pointIndex in xrange(pointCount):
      point = points.GetPoint(pointIndex)
      ijkFloat = rasToIJK.MultiplyPoint(point+(1,))[:3]
      ijk = [int(round(element)) for element in ijkFloat]
      ijk.reverse()
      try:
        labelArray[tuple(ijk)] = labelValue
      except IndexError:
        pass
    labelNode.GetImageData().Modified()
    labelNode.Modified()
    print('finished')


class FiberBundleToLabelMapTest(unittest.TestCase):
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
    self.test_FiberBundleToLabelMap1()

  def test_FiberBundleToLabelMap1(self):
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

    self.applicationLogic = slicer.app.applicationLogic()
    self.volumesLogic = slicer.modules.volumes.logic()

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        ('http://slicer.kitware.com/midas3/download?items=5768', 'tract1.vtk', slicer.util.loadFiberBundle),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    volumeNode = slicer.util.getNode(pattern="FA")
    fiberNode = slicer.util.getNode(pattern="tract1")
    logic = FiberBundleToLabelMapLogic()

    labelNode = slicer.util.getNode(pattern="FA-label")
    if not labelNode:
      labelNode = self.volumesLogic.CreateLabelVolume( slicer.mrmlScene, volumeNode, "FA-label" )


    selectionNode = self.applicationLogic.GetSelectionNode()
    selectionNode.SetReferenceActiveVolumeID( volumeNode.GetID() )
    selectionNode.SetReferenceActiveLabelVolumeID( labelNode.GetID() )
    self.applicationLogic.PropagateVolumeSelection(0)

    logic.rasterizeFibers(fiberNode, labelNode, labelValue=10)

    self.assertTrue( logic.hasImageData(labelNode) )
    self.delayDisplay('Test passed!')
