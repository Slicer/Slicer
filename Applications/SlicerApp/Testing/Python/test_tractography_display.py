import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# TestTractographyDisplay
#

class test_tractography_display:
  def __init__(self, parent):
    parent.title = "Test Tractography Display" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Demian Wassermann (Brigham and Women's Hospital)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    """
    parent.acknowledgementText = """
    This file was originally developed by Demian Wassermann
""" # replace with organization, grant and thanks.
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['test_tractography_display'] = self.runTest

  def runTest(self):
    tester = test_tractography_displayTest()
    tester.runTest()

#
# qTestTractographyDisplayWidget
#

class test_tractography_displayWidget:
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
    self.reloadButton.name = "test_tractography_display Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onReload(self, moduleName="test_tractography_display"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    globals()[moduleName] = slicer.util.reloadScriptedModule(moduleName)

  def onReloadAndTest(self,moduleName="test_tractography_display"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# TestTractographyDisplayLogic
#

class TestTractographyDisplayLogic:
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


class test_tractography_displayTest(unittest.TestCase):
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
    #
    # first, get some data
    #
    import urllib
    downloads = (
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

    # use the volumes module to replicate the bug report
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('TractographyDisplay')
    self.delayDisplay("Entered Tractography module")

  def tearDown(self):
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_TestWindowLevel(display_node='Line')
    self.test_TestWindowLevel(display_node='Tube')
    self.test_TestWindowLevel(display_node='Glyph')
    self.test_TestTractographyDisplayColorBy(display_node='Line')
    self.test_TestTractographyDisplayColorBy(display_node='Tube')
    self.test_TestTractographyDisplayColorBy(display_node='Glyph')
    self.tearDown()

  def test_TestTractographyDisplayColorBy(self, display_node='Tube'):
    self.delayDisplay("Starting the test")

    self.delayDisplay('Showing Advanced Display\n')
    advancedDisplay = slicer.util.findChildren(text='Advanced Display')[0]
    advancedDisplay.collapsed = False

    self.delayDisplay('Selecting tract1\n')
    tractNode = slicer.util.getNode('tract1')
    displayNode = getattr(tractNode, 'Get%sDisplayNode' % display_node)()
    tree = slicer.util.findChildren(name='TractographyDisplayTreeView')[0]
    model = tree.model()
    modelIndex = model.indexFromMRMLNode(tractNode)
    tree.setCurrentIndex(modelIndex)

    tubeTab = slicer.util.findChildren(advancedDisplay, name='%sTab' % display_node)[0]

    visibility = slicer.util.findChildren(tubeTab, text='Visibility')[0]

    color_by_strategies = [
        ('Solid Color', displayNode.colorModeSolid),
        ('Of Tensor Property', displayNode.colorModeScalar),
#        (, displayNode.colorModeFunctionOfScalar),
#        ('Color Fibers By Mean Orientation', displayNode.colorModeUseCellScalars),
        ('Of Scalar Value', displayNode.colorModeScalarData),
        ('Color Fibers By Segment Orientation', displayNode.colorModePointFiberOrientation),
        ('Color Fibers By Mean Orientation', displayNode.colorModeMeanFiberOrientation),
    ]
    for widget_text, color_code in color_by_strategies:

        if not visibility.checked:
            visibility.click()

        solidColor = slicer.util.findChildren(tubeTab, text=widget_text)
        if len(solidColor) == 0:
            raise ValueError("Widget %s not found" % widget_text)
        else:
            solidColor = solidColor[0]
        solidColor.click()

        if displayNode and displayNode.GetColorMode() != color_code:
            self.delayDisplay('Setting Color Mode To %s Did not Work' % widget_text)
            raise Exception()
        else:
            self.delayDisplay("Setting Color Mode To %s Worked" % widget_text)

    visibility.click()

  def test_TestWindowLevel(self, display_node='Tube'):
    self.delayDisplay("Starting the test Window Level")

    self.delayDisplay('Showing Advanced Display\n')
    advancedDisplay = slicer.util.findChildren(text='Advanced Display')[0]
    advancedDisplay.collapsed = False

    self.delayDisplay('Selecting tract1\n')
    tractNode = slicer.util.getNode('tract1')
    displayNode = getattr(tractNode, 'Get%sDisplayNode' % display_node)()
    tree = slicer.util.findChildren(name='TractographyDisplayTreeView')[0]
    model = tree.model()
    modelIndex = model.indexFromMRMLNode(tractNode)
    tree.setCurrentIndex(modelIndex)

    tubeTab = slicer.util.findChildren(advancedDisplay, name='%sTab' % display_node)[0]
    slicer.util.findChildren(tubeTab, text='Of Tensor Property')[0].click()


    visibility = slicer.util.findChildren(tubeTab, text='Visibility')[0]
    if not visibility.checked:
        visibility.click()

    autoWL = slicer.util.findChildren(tubeTab, text='Auto W/L')[0]

    assert(not autoWL.checked or displayNode.GetAutoScalarRange())
    assert(autoWL.checked or not displayNode.GetAutoScalarRange())
    self.delayDisplay('Default Window Level Agrees with Display Node\n')

    autoWL.click()
    assert(not autoWL.checked or displayNode.GetAutoScalarRange())
    assert(autoWL.checked or not displayNode.GetAutoScalarRange())
    self.delayDisplay('Changes in Window Level Agrees with Display Node\n')

    if autoWL.checked:
        autoWL.click()

    slider = slicer.util.findChildren(tubeTab, name='FiberBundleColorRangeWidget')[0]
    slider.setMinimumValue(.1)
    slider.setMaximumValue(.8)
    scalar_range = displayNode.GetScalarRange()
    assert((scalar_range[0] == .1) and (scalar_range[1] == .8))
    self.delayDisplay('Changes in Window Level Values Agree with Display Node\n')

    autoWL.click()
    visibility.click()
