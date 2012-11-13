import os
import unittest
import qt, vtk
from __main__ import slicer

#
# DataProbe
#

class DataProbe:
  def __init__(self, parent):
    import string
    parent.title = "DataProbe"
    parent.categories = ["Quantification"]
    parent.contributors = ["Steve Pieper (Isomics)"]
    parent.helpText = string.Template("""
The DataProbe module is used to get information about the current RAS position being indicated by the mouse position.  See <a href=\"$a/Documentation/$b.$c/Modules/DataProbe\">$a/Documentation/$b.$c/Modules/DataProbe</a> for more information.
    """).substitute({ 'a':parent.slicerWikiUrl, 'b':slicer.app.majorVersion, 'c':slicer.app.minorVersion })
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
    """
    # TODO: need a DataProbe icon
    #parent.icon = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
    self.parent = parent
    self.infoWidget = 0

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # Trigger the menu to be added when application has started up
    if not slicer.app.commandOptions().noMainWindow :
      qt.QTimer.singleShot(0, self.addView);

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['DataProbe'] = self.runTest

  def runTest(self):
    tester = DataProbeTest()
    tester.runTest()

  def __del__(self):
    if self.infoWidget:
      self.infoWidget.removeObservers()

  def addView(self):
    """
    Create the persistent widget shown in the bottom left of the user interface
    Do this in a singleShot callback so the rest of the interface is already
    built.
    """
    # TODO - the parent name will likely change
    try:
      parent = slicer.util.findChildren(text='Data Probe')[0]
    except IndexError:
      print("No Data Probe frame - cannot create DataProbe")
      return
    self.infoWidget = DataProbeInfoWidget(parent,type='small')
    parent.layout().insertWidget(0,self.infoWidget.frame)

class DataProbeInfoWidget(object):

  def __init__(self, parent=None,type='small'):
    self.type = type
    self.nameSize = 24

    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

    layoutManager = slicer.app.layoutManager()
    layoutManager.connect('layoutChanged(int)', self.refreshObservers)

    self.frame = qt.QFrame(parent)
    self.frame.setLayout(qt.QVBoxLayout())
    if type == 'small':
      self.createSmall()

    #Helper class to calculate and display tensor scalars
    self.calculateTensorScalars = CalculateTensorScalars()


  def __del__(self):
    self.removeObservers()

  def fitName(self,name):
    if len(name) > self.nameSize:
      preSize = self.nameSize / 2
      postSize = preSize - 3
      name = name[:preSize] + "..." + name[-postSize:]
    return name


  def removeObservers(self):
    # remove observers and reset
    for observee,tag in self.styleObserverTags:
      observee.RemoveObserver(tag)
    self.styleObserverTags = []
    self.sliceWidgetsPerStyle = {}

  def refreshObservers(self):
    """ When the layout changes, drop the observers from
    all the old widgets and create new observers for the
    newly created widgets"""
    self.removeObservers()
    # get new slice nodes
    layoutManager = slicer.app.layoutManager()
    sliceNodeCount = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLSliceNode')
    for nodeIndex in xrange(sliceNodeCount):
      # find the widget for each node in scene
      sliceNode = slicer.mrmlScene.GetNthNodeByClass(nodeIndex, 'vtkMRMLSliceNode')
      sliceWidget = layoutManager.sliceWidget(sliceNode.GetLayoutName())
      if sliceWidget:
        # add obserservers and keep track of tags
        style = sliceWidget.sliceView().interactorStyle()
        self.sliceWidgetsPerStyle[style] = sliceWidget
        events = ("MouseMoveEvent", "EnterEvent", "LeaveEvent")
        for event in events:
          tag = style.AddObserver(event, self.processEvent)
          self.styleObserverTags.append([style,tag])
      # TODO: also observe the slice nodes

  def getPixelString(self,volumeNode,ijk):
    """Given a volume node, create a human readable
    string describing the contents"""
    # TODO: the volume nodes should have a way to generate
    # these strings in a generic way
    if not volumeNode:
      return "No volume"
    imageData = volumeNode.GetImageData()
    if not imageData:
      return "No Image"
    dims = imageData.GetDimensions()
    for ele in xrange(3):
      if ijk[ele] < 0 or ijk[ele] >= dims[ele]:
        return "Out of Frame"
    pixel = ""
    if volumeNode.GetLabelMap():
      labelIndex = int(imageData.GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], 0))
      labelValue = "Unknown"
      colorNode = volumeNode.GetDisplayNode().GetColorNode()
      if colorNode:
        labelValue = colorNode.GetColorName(labelIndex)
      return "%s (%d)" % (labelValue, labelIndex)

    if volumeNode.IsA("vtkMRMLDiffusionTensorVolumeNode"):
        point_idx = imageData.FindPoint(ijk[0], ijk[1], ijk[2])
        if point_idx == -1:
            return "Out of bounds"

        if not imageData.GetPointData():
            return "No Point Data"

        tensors = imageData.GetPointData().GetTensors()
        if not tensors:
            return "No Tensor Data"

        tensor = imageData.GetPointData().GetTensors().GetTuple9(point_idx)
        scalarVolumeDisplayNode = volumeNode.GetScalarVolumeDisplayNode()

        if scalarVolumeDisplayNode:
            operation = scalarVolumeDisplayNode.GetScalarInvariant()
        else:
            operation = None

        value = self.calculateTensorScalars(tensor, operation=operation)
        if value is not None:
            valueString = ("%f" % value).rstrip('0').rstrip('.')
            return "%s %s"%(scalarVolumeDisplayNode.GetScalarInvariantAsString(), valueString)
        else:
            return scalarVolumeDisplayNode.GetScalarInvariantAsString()

    # default - non label scalar volume
    numberOfComponents = imageData.GetNumberOfScalarComponents()
    if numberOfComponents > 3:
      return "%d components" % numberOfComponents
    for c in xrange(numberOfComponents):
      component = imageData.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
      if component.is_integer():
        component = int(component)
      # format string according to suggestion here:
      # http://stackoverflow.com/questions/2440692/formatting-floats-in-python-without-superfluous-zeros
      componentString = ("%f" % component).rstrip('0').rstrip('.')
      pixel += ("%s, " % componentString)
    return pixel[:-2]


  def processEvent(self,observee,event):
    # TODO: use a timer to delay calculation and compress events
    if event == 'LeaveEvent':
      # reset all the readouts
      self.viewerColor.setText( "" )
      self.viewerName.setText( "" )
      self.viewerRAS.setText( "" )
      self.viewerOrient.setText( "" )
      self.viewerSpacing.setText( "" )
      layers = ('L', 'F', 'B')
      for layer in layers:
        self.layerNames[layer].setText( "" )
        self.layerIJKs[layer].setText( "" )
        self.layerValues[layer].setText( "" )
      return
    if self.sliceWidgetsPerStyle.has_key(observee):
      sliceWidget = self.sliceWidgetsPerStyle[observee]
      sliceLogic = sliceWidget.sliceLogic()
      sliceNode = sliceWidget.mrmlSliceNode()
      interactor = observee.GetInteractor()
      xy = interactor.GetEventPosition()
      xyz = sliceWidget.sliceView().convertDeviceToXYZ(xy);
      # populate the widgets
      self.viewerColor.setText( " " )
      rgbColor = sliceNode.GetLayoutColor();
      color = qt.QColor.fromRgbF(rgbColor[0], rgbColor[1], rgbColor[2])
      if hasattr(color, 'name'):
        self.viewerColor.setStyleSheet('QLabel {background-color : %s}' % color.name())
      self.viewerName.setText( "  " + sliceNode.GetLayoutName() + "  " )
      # TODO: get z value from lightbox
      ras = sliceWidget.sliceView().convertXYZToRAS(xyz)
      self.viewerRAS.setText( "RAS: (%.1f, %.1f, %.1f)" % ras )
      self.viewerOrient.setText( "  " + sliceWidget.sliceOrientation )
      self.viewerSpacing.setText( "%.1f" % sliceLogic.GetLowestVolumeSliceSpacing()[2] )
      if sliceNode.GetSliceSpacingMode() == 1:
        self.viewerSpacing.setText( "(" + self.viewerSpacing.text + ")" )
      self.viewerSpacing.setText( " Sp: " + self.viewerSpacing.text )
      layerLogicCalls = (('L', sliceLogic.GetLabelLayer),
                         ('F', sliceLogic.GetForegroundLayer),
                         ('B', sliceLogic.GetBackgroundLayer))
      for layer,logicCall in layerLogicCalls:
        layerLogic = logicCall()
        volumeNode = layerLogic.GetVolumeNode()
        nameLabel = "None"
        ijkLabel = ""
        valueLabel = ""
        if volumeNode:
          nameLabel = self.fitName(volumeNode.GetName())
          xyToIJK = layerLogic.GetXYToIJKTransform().GetMatrix()
          ijkFloat = xyToIJK.MultiplyPoint(xyz+(1,))[:3]
          ijk = []
          for element in ijkFloat:
            try:
              index = int(round(element))
            except ValueError:
              index = 0
            ijk.append(index)
            ijkLabel += "%d, " % index
          ijkLabel = ijkLabel[:-2]
          valueLabel = self.getPixelString(volumeNode,ijk)
        self.layerNames[layer].setText( '<b>' + nameLabel )
        self.layerIJKs[layer].setText( '(' + ijkLabel + ')' )
        self.layerValues[layer].setText( '<b>' + valueLabel )

  def createSmall(self):
    """Make the internals of the widget to display in the
    Data Probe frame (lower left of slicer main window by default)"""
    # top row - things about the viewer itself
    self.viewerFrame = qt.QFrame(self.frame)
    self.viewerFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.viewerFrame)
    self.viewerColor = qt.QLabel(self.viewerFrame)
    self.viewerFrame.layout().addWidget(self.viewerColor)
    self.viewerName = qt.QLabel(self.viewerFrame)
    self.viewerFrame.layout().addWidget(self.viewerName)
    self.viewerRAS = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerRAS)
    self.viewerOrient = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerOrient)
    self.viewerSpacing = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerSpacing)
    self.viewerFrame.layout().addStretch(1)

    # the grid - things about the layers
    # this method makes labels
    self.layerGrid = qt.QFrame(self.frame)
    self.layerGrid.setLayout(qt.QGridLayout())
    self.frame.layout().addWidget(self.layerGrid)
    layers = ('L', 'F', 'B')
    self.layerNames = {}
    self.layerIJKs = {}
    self.layerValues = {}
    row = 0
    for layer in layers:
      col = 0
      self.layerGrid.layout().addWidget(qt.QLabel(layer), row, col)
      col += 1
      self.layerNames[layer] = qt.QLabel()
      self.layerGrid.layout().addWidget(self.layerNames[layer], row, col)
      col += 1
      self.layerIJKs[layer] = qt.QLabel()
      self.layerGrid.layout().addWidget(self.layerIJKs[layer], row, col)
      col += 1
      self.layerValues[layer] = qt.QLabel()
      self.layerGrid.layout().addWidget(self.layerValues[layer], row, col)
      self.layerGrid.layout().setColumnStretch(col,100)
      col += 1
      row += 1

    # goto module button
    self.goToModule = qt.QPushButton('->', self.frame)
    self.goToModule.setToolTip('Go to the DataProbe module for more information and options')
    self.frame.layout().addWidget(self.goToModule)
    self.goToModule.connect("clicked()", self.onGoToModule)
    # hide this for now - there's not much to see in the module itself
    self.goToModule.hide()

  def onGoToModule(self):
    m = slicer.util.mainWindow()
    m.moduleSelector().selectModule('DataProbe')

#
# DataProbe widget
#

class DataProbeWidget:
  """This builds the module contents - nothing here"""
  # TODO: this could have a more in-depth set of information
  # about the volumes and layers in the slice views
  # and possibly other view types as well
  # TODO: Since this is empty for now, it should be hidden
  # from the Modules menu.

  def __init__(self, parent=None):
    self.observerTags = []

    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
      self.layout = self.parent.layout()
      self.setup()
      self.parent.show()
    else:
      self.parent = parent
      self.layout = parent.layout()

  def enter(self):
    pass

  def exit(self):
    pass

  def updateGUIFromMRML(self, caller, event):
    pass

  def setup(self):

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "DataProbe Reload"
    self.layout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    self.layout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

    self.parent.layout().addStretch(1)

  def onReload(self,moduleName="DataProbe"):
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

  def onReloadAndTest(self,moduleName="DataProbe"):
    self.onReload()
    evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
    tester = eval(evalString)
    tester.runTest()


class CalculateTensorScalars:
    def __init__(self):
        self.dti_math = slicer.vtkDiffusionTensorMathematics()

        self.single_pixel_image = vtk.vtkImageData()
        self.single_pixel_image.SetExtent(0, 0, 0, 0, 0, 0)
        self.single_pixel_image.AllocateScalars()

        self.tensor_data = vtk.vtkFloatArray()
        self.tensor_data.SetNumberOfComponents(9)
        self.tensor_data.SetNumberOfTuples(self.single_pixel_image.GetNumberOfPoints())
        self.single_pixel_image.GetPointData().SetTensors(self.tensor_data)

        self.dti_math.SetInput(self.single_pixel_image)

    def __call__(self, tensor, operation=None):
        if len(tensor) != 9:
            raise ValueError("Invalid tensor a 9-array is required")

        self.tensor_data.SetTupleValue(0, tensor)
        self.tensor_data.Modified()
        self.single_pixel_image.Modified()

        if operation is not None:
            self.dti_math.SetOperation(operation)
        else:
            self.dti_math.SetOperationToFractionalAnisotropy()

        self.dti_math.Update()
        output = self.dti_math.GetOutput()

        if output and output.GetNumberOfScalarComponents() > 0:
            value = output.GetScalarComponentAsDouble(0, 0, 0, 0)
            return value
        else:
            return None


#
# DataProbeLogic
#

class DataProbeLogic:
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


class DataProbeTest(unittest.TestCase):
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
    pass

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_DataProbe1()

  def test_DataProbe1(self):
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
    if not slicer.util.getNode('FA'):
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
    self.delayDisplay('Finished with download and loading\n')

    self.widget = DataProbeInfoWidget()
    self.widget.frame.show()

    self.delayDisplay('Test passed!')
