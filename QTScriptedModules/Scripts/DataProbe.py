import os
from __main__ import slicer
import qt, ctk

#
# DataProbe
#

class DataProbe:
  def __init__(self, parent):
    parent.title = "DataProbe"
    parent.category = "Quantification"
    parent.contributor = "Steve Pieper"
    parent.helpText = """
The DataProbe module is used to get information about the current RAS position being indicated by the mouse position.
    """
    parent.acknowledgementText = """
This work is supported by NA-MIC, NAC, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details.  Module implemented by Steve Pieper.
    """
    # TODO: need a DataProbe icon
    #parent.icon = qt.QIcon(':Icons/XLarge/SlicerDownloadMRHead.png')
    self.parent = parent

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # Trigger the menu to be added when application has started up
    qt.QTimer.singleShot(0, self.addView);
    

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
    infoWidget = DataProbeInfoWidget(parent,type='small')
    parent.layout().insertWidget(0,infoWidget.frame)
    globals()['infoWidget'] = infoWidget


class DataProbeInfoWidget(object):

  def __init__(self, parent=None,type='small'):
    self.type = type

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


  def refreshObservers(self):
    for observee,tag in self.styleObserverTags:
      observee.RemoveObserver(tag)
    self.sliceWidgetsPerStyle = {}
    layoutManager = slicer.app.layoutManager()
    sliceNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLSliceNode')
    sliceNodes.InitTraversal()
    for nodeIndex in xrange(sliceNodes.GetNumberOfItems()):
      sliceNode = sliceNodes.GetNextItemAsObject()
      sliceWidget = layoutManager.sliceWidget(sliceNode.GetLayoutName())
      if sliceWidget:
        style = sliceWidget.sliceView().interactorStyle()
        self.sliceWidgetsPerStyle[style] = sliceWidget
        events = ("MouseMoveEvent", "EnterEvent", "LeaveEvent")
        for event in events:
          tag = style.AddObserver(event, self.processEvent)
          self.styleObserverTags.append([style,tag])

  def getPixelString(self,volumeNode,ijk):
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
      labelIndex = imageData.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],0)
      labelValue = "Unknown"
      colorNode = volumeNode.GetDisplayNode().GetColorNode()
      if colorNode:
        labelValue = colorNode.GetColorName(labelIndex)
      return "%s (%d)" % (labelValue, labelIndex)
    for c in xrange(imageData.GetNumberOfScalarComponents()):
      component = imageData.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
      if component == int(component):
        component = int(component)
      pixel += "%d, " % component
    return pixel[:-2] 


  def processEvent(self,observee,event):   
    if event == 'LeaveEvent':
      self.frame.hide()
    else:
      self.frame.show()
    if self.sliceWidgetsPerStyle.has_key(observee):
      sliceWidget = self.sliceWidgetsPerStyle[observee]
      sliceLogic = sliceWidget.sliceLogic()
      sliceNode = sliceWidget.mrmlSliceNode()
      interactor = observee.GetInteractor()
      xy = interactor.GetEventPosition()
      # populate the widgets
      self.viewerColor.text = " "
      self.viewerColor.setStyleSheet('QLabel {background-color : %s}' % sliceNode.GetLayoutName())
      self.viewerName.text = sliceNode.GetLayoutName()
      # TODO: get z value from lightbox
      ras = sliceNode.GetXYToRAS().MultiplyPoint(xy+(0,1))[:3]
      self.viewerRAS.text = "RAS: %.1f, %.1f, %.1f" % ras
      self.viewerOrient.text = sliceWidget.sliceOrientation
      self.viewerSpacing.text = "%.1f" % sliceLogic.GetLowestVolumeSliceSpacing()[2]
      if sliceNode.GetSliceSpacingMode() == 1:
        self.viewerSpacing.text = "(" + self.viewerSpacing.text + ")"
      self.viewerSpacing.text = "Sp: " + self.viewerSpacing.text
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
          nameLabel = volumeNode.GetName()
          xyToIJK = layerLogic.GetXYToIJKTransform().GetMatrix()
          ijkFloat = xyToIJK.MultiplyPoint(xy+(0,1))[:3]
          ijk = []
          for element in ijkFloat:
            index = int(round(element))
            ijk.append(index)
            ijkLabel += "%d, " % index
          ijkLabel = ijkLabel[:-2]
          valueLabel = self.getPixelString(volumeNode,ijk)
        self.layerNames[layer].text = nameLabel
        self.layerIJKs[layer].text = ijkLabel
        self.layerValues[layer].text = valueLabel

  def createSmall(self):

    # top row - things about the viewer itself
    self.viewerFrame = qt.QFrame(self.frame)
    self.viewerFrame.setLayout(qt.QHBoxLayout())
    self.frame.layout().addWidget(self.viewerFrame)
    self.viewerColor = qt.QLabel(self.viewerFrame)
    self.viewerFrame.layout().addWidget(self.viewerColor)
    self.viewerFrame.layout().addStretch(1)
    self.viewerName = qt.QLabel(self.viewerFrame)
    self.viewerFrame.layout().addWidget(self.viewerName)
    self.viewerFrame.layout().addStretch(1)
    self.viewerRAS = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerRAS)
    self.viewerFrame.layout().addStretch(1)
    self.viewerOrient = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerOrient)
    self.viewerFrame.layout().addStretch(1)
    self.viewerSpacing = qt.QLabel()
    self.viewerFrame.layout().addWidget(self.viewerSpacing)
    self.viewerFrame.layout().addStretch(1)

    # the grid - things about the layers
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
    self.parent.layout().addWidget(qt.QLabel("hoot!"))
