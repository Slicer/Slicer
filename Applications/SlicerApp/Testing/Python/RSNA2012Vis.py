import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# RSNA2012Vis
#

class RSNA2012Vis:
  def __init__(self, parent):
    parent.title = "RSNA2012Vis" # TODO make this more human readable by adding spaces
    parent.categories = ["Testing.TestCases"]
    parent.dependencies = []
    parent.contributors = ["Steve Pieper (Isomics)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    This module was developed as a self test to perform the operations needed for the RSNA 2012 Visualization Tutorial
    """
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
    slicer.selfTests['RSNA2012Vis'] = self.runTest

  def runTest(self):
    tester = RSNA2012VisTest()
    tester.runTest()

#
# qRSNA2012VisWidget
#

class RSNA2012VisWidget:
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
    self.reloadButton.name = "RSNA2012Vis Reload"
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
    testsCollapsibleButton = ctk.ctkCollapsibleButton()
    testsCollapsibleButton.text = "A collapsible button"
    self.layout.addWidget(testsCollapsibleButton)

    # Layout within the collapsible button
    formLayout = qt.QFormLayout(testsCollapsibleButton)

    # test buttons
    tests = ( ("Part 1: DICOM",self.onPart1DICOM),("Part 2: Head", self.onPart2Head),("Part 3: Liver", self.onPart3Liver),("Part 4: Lung", self.onPart4Lung),)
    for text,slot in tests:
      testButton = qt.QPushButton(text)
      testButton.toolTip = "Run the test."
      formLayout.addWidget(testButton)
      testButton.connect('clicked(bool)', slot)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onPart1DICOM(self):
    tester = RSNA2012VisTest()
    tester.setUp()
    tester.test_Part1DICOM()

  def onPart2Head(self):
    tester = RSNA2012VisTest()
    tester.setUp()
    tester.test_Part2Head()

  def onPart3Liver(self):
    tester = RSNA2012VisTest()
    tester.setUp()
    tester.test_Part3Liver()

  def onPart4Lung(self):
    tester = RSNA2012VisTest()
    tester.setUp()
    tester.test_Part4Lung()

  def onReload(self,moduleName="RSNA2012Vis"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    import imp, sys, os, slicer

    widgetName = moduleName + "Widget"

    # reload the source code
    # - set source file path
    # - load the module to the global space
    filePath = eval('slicer.modules.%s.path' % moduleName.lower())
    p = os.path.dirname(filePath)
    if not sys.path.__contains__(p):
      sys.path.insert(0,p)
    fp = open(filePath, "r")
    globals()[moduleName] = imp.load_module(
        moduleName, fp, filePath, ('.py', 'r', imp.PY_SOURCE))
    fp.close()

    # rebuild the widget
    # - find and hide the existing widget
    # - create a new widget in the existing parent
    parent = slicer.util.findChildren(name='%s Reload' % moduleName)[0].parent()
    for child in parent.children():
      try:
        child.hide()
      except AttributeError:
        pass
    # Remove spacer items
    item = parent.layout().itemAt(0)
    while item:
      parent.layout().removeItem(item)
      item = parent.layout().itemAt(0)
    # create new widget inside existing parent
    globals()[widgetName.lower()] = eval(
        'globals()["%s"].%s(parent)' % (moduleName, widgetName))
    globals()[widgetName.lower()].setup()

  def onReloadAndTest(self,moduleName="RSNA2012Vis"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()

#
# RSNA2012VisLogic
#

class RSNA2012VisLogic:
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


class RSNA2012VisTest(unittest.TestCase):
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
    self.delayDisplay("Closing the scene")
    layoutManager = slicer.app.layoutManager()
    layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
    slicer.mrmlScene.Clear(0)

  def clickAndDrag(self,widget,button='Left',start=(10,10),end=(10,40),steps=20,modifiers=[]):
    """Send synthetic mouse events to the specified widget (qMRMLSliceWidget or qMRMLThreeDView)
    button : "Left", "Middle", "Right", or "None"
    start, end : window coordinates for action
    steps : number of steps to move in
    modifiers : list containing zero or more of "Shift" or "Control"
    """
    style = widget.interactorStyle()
    interator = style.GetInteractor()
    if button == 'Left':
      down = style.OnLeftButtonDown
      up = style.OnLeftButtonUp
    elif button == 'Right':
      down = style.OnRightButtonDown
      up = style.OnRightButtonUp
    elif button == 'Middle':
      down = style.OnMiddleButtonDown
      up = style.OnMiddleButtonUp
    elif button == 'None' or not button:
      down = lambda : None
      up = lambda : None
    else:
      raise Exception("Bad button - should be Left or Right, not %s" % button)
    if 'Shift' in modifiers:
      interator.SetShiftKey(1)
    if 'Control' in modifiers:
      interator.SetControlKey(1)
    interator.SetEventPosition(*start)
    down()
    for step in xrange(steps):
      frac = float(step)/steps
      x = start[0] + frac*(end[0]-start[0])
      y = start[1] + frac*(end[1]-start[1])
      interator.SetEventPosition(x,y)
      style.OnMouseMove()
    up()
    interator.SetShiftKey(0)
    interator.SetControlKey(0)


  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_Part1DICOM()
    self.setUp()
    self.test_Part2Head()
    self.setUp()
    self.test_Part3Liver()
    self.setUp()
    self.test_Part4Lung()

  def test_Part1DICOM(self):
    """ Test the DICOM part of the test using the head atlas
    """

    import os
    self.delayDisplay("Starting the DICOM test")
    #
    # first, get the data - a zip file of dicom data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=8610', 'dicom.zip'),
        )

    self.delayDisplay("Downloading")
    for url,name in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        self.delayDisplay('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
    self.delayDisplay('Finished with download\n')

    self.delayDisplay("Unzipping")
    dicomFilesDirectory = slicer.app.temporaryPath + '/dicomFiles'
    qt.QDir().mkpath(dicomFilesDirectory)
    slicer.app.applicationLogic().Unzip(filePath, dicomFilesDirectory)

    self.delayDisplay("Switching to temp database directory")
    if slicer.dicomDatabase:
      originalDatabaseDirectory = os.path.split(slicer.dicomDatabase.databaseFilename)[0]
    else:
      originalDatabaseDirectory = None
    tempDatabaseDirectory = slicer.app.temporaryPath + '/tempDICOMDatbase'
    qt.QDir().mkpath(tempDatabaseDirectory)
    dicomWidget = slicer.modules.dicom.widgetRepresentation().self()
    dicomWidget.onDatabaseDirectoryChanged(tempDatabaseDirectory)

    try:
      logic = RSNA2012VisLogic()

      self.delayDisplay('Importing DICOM')
      mainWindow = slicer.util.mainWindow()
      mainWindow.moduleSelector().selectModule('DICOM')
      dicomWidget.dicomApp.suspendModel()
      indexer = ctk.ctkDICOMIndexer()
      indexer.addDirectory(slicer.dicomDatabase, dicomFilesDirectory, None)
      dicomWidget.dicomApp.resumeModel()
      dicomWidget.detailsPopup.open()
      # click on the first row of the tree
      index = dicomWidget.tree.indexAt(qt.QPoint(0,0))
      dicomWidget.onTreeClicked(index)

      self.delayDisplay('Loading Selection')
      dicomWidget.detailsPopup.loadCheckedLoadables()

      self.delayDisplay('Change Level')
      layoutManager = slicer.app.layoutManager()
      redWidget = layoutManager.sliceWidget('Red')
      self.clickAndDrag(redWidget,start=(10,10),end=(10,40))

      self.delayDisplay('Change Window')
      self.clickAndDrag(redWidget,start=(10,10),end=(40,10))

      self.delayDisplay('Change Layout')
      layoutManager = slicer.app.layoutManager()
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)

      self.delayDisplay('Zoom')
      self.clickAndDrag(redWidget,button='Right',start=(10,10),end=(10,40))

      self.delayDisplay('Pan')
      self.clickAndDrag(redWidget,button='Middle',start=(10,10),end=(40,40))

      self.delayDisplay('Center')
      redWidget.sliceController().fitSliceToBackground()

      self.delayDisplay('Lightbox')
      redWidget.sliceController().setLightboxTo6x6()

      self.delayDisplay('Conventional Layout')
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)

      self.delayDisplay('No Lightbox')
      redWidget.sliceController().setLightboxTo1x1()

      self.delayDisplay('Four Up Layout')
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutFourUpView)

      self.delayDisplay('Shift Mouse')
      self.clickAndDrag(redWidget,button='None',start=(100,100),end=(140,140),modifiers=['Shift'])

      self.delayDisplay('Conventional, Link, Slice Model')
      layoutManager.setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutConventionalView)
      redWidget.sliceController().setSliceLink(True)
      redWidget.sliceController().setSliceVisible(True);

      self.delayDisplay('Rotate')
      threeDView = layoutManager.threeDWidget(0).threeDView()
      self.clickAndDrag(threeDView)

      self.delayDisplay('Zoom')
      threeDView = layoutManager.threeDWidget(0).threeDView()
      self.clickAndDrag(threeDView,button='Right')

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

    self.delayDisplay("Restoring original database directory")
    if originalDatabaseDirectory:
      dicomWidget.onDatabaseDirectoryChanged(originalDatabaseDirectory)

  def test_Part2Head(self):
    """ Test using the head atlas - may not be needed - Slicer4Minute is already tested
    """
    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=8609', '3DHeadData.mrb', slicer.util.loadScene),
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

    try:
      logic = RSNA2012VisLogic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      greenWidget = layoutManager.sliceWidget('Green')
      greenController = greenWidget.sliceController()

      self.delayDisplay('Models and Slice Model')
      mainWindow.moduleSelector().selectModule('Models')
      redWidget.sliceController().setSliceVisible(True);

      self.delayDisplay('Scroll Slices')
      for offset in xrange(-20,20,2):
        redController.setSliceOffsetValue(offset)

      self.delayDisplay('Skin Opacity')
      # turn off skin and skull
      skin = slicer.util.getNode(pattern='Skin.vtk')
      skin.GetDisplayNode().SetOpacity(0.5)

      self.delayDisplay('Skin and Skull Visibility')
      skin.GetDisplayNode().SetVisibility(0)
      skull = slicer.util.getNode(pattern='skull_bone.vtk')
      skull.GetDisplayNode().SetVisibility(0)

      self.delayDisplay('Green slice and Clipping')
      greenWidget.sliceController().setSliceVisible(True);
      hemispheric_white_matter = slicer.util.getNode(pattern='hemispheric_white_matter.vtk')
      hemispheric_white_matter.GetDisplayNode().SetClipping(1)
      clip = slicer.util.getNode(pattern='vtkMRMLClipModelsNode1')
      clip.SetRedSliceClipState(0)
      clip.SetYellowSliceClipState(0)
      clip.SetGreenSliceClipState(2)

      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.mrmlScene.GetNodesByClass('vtkMRMLCameraNode')
      for index in xrange(cameras.GetNumberOfItems()):
        cameraNode = cameras.GetItemAsObject(index)
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break
      cameraNode.GetCamera().Azimuth(90)
      cameraNode.GetCamera().Elevation(20)

      self.delayDisplay('Rotate')
      self.clickAndDrag(threeDView)

      self.delayDisplay('Zoom')
      threeDView = layoutManager.threeDWidget(0).threeDView()
      self.clickAndDrag(threeDView,button='Right')

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))


  def test_Part3Liver(self):
    """ Test using the liver example data
    """
    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=8611', 'LiverData.mrb', slicer.util.loadScene),
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

    try:
      logic = RSNA2012VisLogic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.mrmlScene.GetNodesByClass('vtkMRMLCameraNode')
      for index in xrange(cameras.GetNumberOfItems()):
        cameraNode = cameras.GetItemAsObject(index)
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break

      self.delayDisplay('Segment II invisible')
      mainWindow.moduleSelector().selectModule('Models')
      segmentII = slicer.util.getNode('LiverSegment_II')
      segmentII.GetDisplayNode().SetVisibility(0)
      self.clickAndDrag(threeDView,start=(10,200),end=(10,10))

      self.delayDisplay('Segment II visible')
      segmentII.GetDisplayNode().SetVisibility(1)
      cameraNode.GetCamera().Azimuth(0)
      cameraNode.GetCamera().Elevation(0)

      self.delayDisplay('View Adrenal')
      segmentII.GetDisplayNode().SetVisibility(1)
      cameraNode.GetCamera().Azimuth(180)
      cameraNode.GetCamera().Elevation(-30)

      segmentVII = slicer.util.getNode('LiverSegment_II')
      redWidget.sliceController().setSliceVisible(True);

      self.delayDisplay('Middle Hepatic')
      models = slicer.mrmlScene.GetNodesByClass('vtkMRMLModelNode')
      for index in xrange(models.GetNumberOfItems()):
        modelNode = models.GetItemAsObject(index)
        modelNode.GetDisplayNode().SetVisibility(0)

      segmentVII = slicer.util.getNode('LiverSegment_II')
      transparentNodes = ('MiddleHepaticVein_and_Branches','LiverSegment_IVb','LiverSegmentV',)
      for nodeName in transparentNodes:
        modelNode = slicer.util.getNode(nodeName)
        modelNode.GetDisplayNode().SetOpacity(0.5)
        modelNode.GetDisplayNode().SetVisibility(1)
      redWidget.sliceController().setSliceVisible(True);
      cameraNode.GetCamera().Azimuth(30)
      cameraNode.GetCamera().Elevation(-20)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))

  def test_Part4Lung(self):
    """ Test using the lung data
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=8612', 'LungData.mrb', slicer.util.loadScene),
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

    try:
      logic = RSNA2012VisLogic()
      mainWindow = slicer.util.mainWindow()
      layoutManager = slicer.app.layoutManager()
      threeDView = layoutManager.threeDWidget(0).threeDView()
      redWidget = layoutManager.sliceWidget('Red')
      redController = redWidget.sliceController()
      viewNode = threeDView.mrmlViewNode()
      cameras = slicer.mrmlScene.GetNodesByClass('vtkMRMLCameraNode')
      for index in xrange(cameras.GetNumberOfItems()):
        cameraNode = cameras.GetItemAsObject(index)
        if cameraNode.GetActiveTag() == viewNode.GetID():
          break

      self.delayDisplay('Reset view')
      threeDView.resetFocalPoint()
      mainWindow.moduleSelector().selectModule('Models')

      self.delayDisplay('View Question 1')
      cameraNode.GetCamera().Azimuth(-100)
      cameraNode.GetCamera().Elevation(-40)
      redWidget.sliceController().setSliceVisible(True);
      lungs = slicer.util.getNode('chestCT_lungs')
      lungs.GetDisplayNode().SetVisibility(0)

      self.delayDisplay('View Question 2')
      cameraNode.GetCamera().Azimuth(-65)
      cameraNode.GetCamera().Elevation(-20)
      lungs.GetDisplayNode().SetVisibility(1)
      lungs.GetDisplayNode().SetOpacity(0.24)
      redController.setSliceOffsetValue(-50)

      self.delayDisplay('View Question 3')
      cameraNode.GetCamera().Azimuth(-165)
      cameraNode.GetCamera().Elevation(-10)
      redWidget.sliceController().setSliceVisible(False);

      self.delayDisplay('View Question 4')
      cameraNode.GetCamera().Azimuth(20)
      cameraNode.GetCamera().Elevation(-10)
      lowerLobeNodes = slicer.util.getNodes('*LowerLobe*')
      for showNode in lowerLobeNodes:
        self.delayDisplay('Showing Node %s' % showNode, 300)
        for node in lowerLobeNodes:
          displayNode = lowerLobeNodes[node].GetDisplayNode()
          if displayNode:
            displayNode.SetVisibility(1 if node == showNode else 0)

      self.delayDisplay('Test passed!')
    except Exception, e:
      import traceback
      traceback.print_exc()
      self.delayDisplay('Test caused exception!\n' + str(e))
