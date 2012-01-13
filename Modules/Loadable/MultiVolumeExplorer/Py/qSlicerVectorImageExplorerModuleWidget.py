from __main__ import vtk, qt, ctk, slicer

from EMSegmentWizard import Helper

class qSlicerVectorImageExplorerModuleWidget:
  def __init__( self, parent=None ):

    print 'My module init called, parent = ', parent
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout( qt.QVBoxLayout() )
    else:
      self.parent = parent
      print 'Layout is ', self.parent.layout()

    # parent.dependencies = []

    self.layout = self.parent.layout()

    # this flag is 1 if there is an update in progress
    self.__updating = 1

    # Reference to the logic
    self.__logic = None

    if not parent:
      self.__logic = slicer.modulelogic.vtkVectorImageExplorerLogic()
      self.setup()
      self.parent.setMRMLScene( slicer.mrmlScene )
      # after setup, be ready for events
      self.__updating = 0

      self.parent.show()

    self.__dwvNode = None

  def setup( self ):
    '''
    Create and start the EMSegment workflow.
    '''
    
    print 'My module Setup called'
    
    # Use the logic associated with the module
    #if not self.__logic:
    #  self.__logic = self.parent.module().logic()

    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)

    w = qt.QWidget()
    layout = qt.QFormLayout()
    w.setLayout(layout)
    self.layout.addWidget(w)
    w.show()
    self.layout = layout
    
    label = qt.QLabel('Input container:')
    self.__vcSelector = slicer.qMRMLNodeComboBox()
    self.__vcSelector.nodeTypes = ['vtkMRMLVectorImageContainerNode']
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.__vcSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)
    self.__vcSelector.addEnabled = 1

    self.__vcSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onInputChanged)

    self.layout.addRow(label, self.__vcSelector)


    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.__mdSlider = ctk.ctkSliderWidget()
    #self.__mdSlider.setRange(0,10)
    #self.__mdSlider.setValue(5)

    label = qt.QLabel('Vector scroller:')
    self.layout.addRow(label, self.__mdSlider)

    self.__mdSlider.connect('valueChanged(double)', self.onSliderChanged)

    label = qt.QLabel('Label for the element shown:')
    self.__mdValue = qt.QLabel()
    self.layout.addRow(label, self.__mdValue)

    label = qt.QLabel('Vector content:')
    self.__vcValue = qt.QLabel()
    self.layout.addRow(label, self.__vcValue)

    # initialize slice observers (from DataProbe.py)
    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

  def onSliderChanged(self, newValue):
    value = self.__mdSlider.value
    self.__mdValue.setText(str(newValue))

    if self.__dwvNode != None:
      dwvDisplayNode = self.__dwvNode.GetDisplayNode()
      dwvDisplayNode.SetDiffusionComponent(newValue)

  def onInputChanged(self):
    vcNode = self.__vcSelector.currentNode()
    if vcNode != None:
       self.__dwvNode = vcNode.GetDWVNode()
       print 'Active DWV node: ', self.__dwvNode
       if self.__dwvNode != None:
         self.__mdSlider.minimum = 0
         self.__mdSlider.maximum = self.__dwvNode.GetNumberOfGradients()-1
    
  def onMRMLSceneChanged(self, mrmlScene):
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    return
    '''
    if mrmlScene != self.__logic.GetMRMLScene():
      self.__logic.SetMRMLScene(mrmlScene)
      self.__logic.RegisterNodes()
      self.__logic.InitializeEventListeners()
    self.__logic.GetMRMLManager().SetMRMLScene(mrmlScene)
    '''
    
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
 
  def processEvent(self,observee,event):
    # TODO: use a timer to delay calculation and compress events
    if event == 'LeaveEvent':
      # reset all the readouts
      # TODO: reset the label text
      return
    if self.sliceWidgetsPerStyle.has_key(observee):
      sliceWidget = self.sliceWidgetsPerStyle[observee]
      sliceLogic = sliceWidget.sliceLogic()
      sliceNode = sliceWidget.mrmlSliceNode()
      interactor = observee.GetInteractor()
      xy = interactor.GetEventPosition()
      xyz = sliceWidget.convertDeviceToXYZ(xy);

      # TODO: get z value from lightbox
      ras = sliceWidget.convertXYZToRAS(xyz)
      '''
      layerLogicCalls = (('L', sliceLogic.GetLabelLayer),
                         ('F', sliceLogic.GetForegroundLayer),
                         ('B', sliceLogic.GetBackgroundLayer))
      '''
      layerLogicCalls = [sliceLogic.GetBackgroundLayer]
      for logicCall in layerLogicCalls:
        layerLogic = logicCall()
        volumeNode = layerLogic.GetVolumeNode()
        nameLabel = "None"
        ijkLabel = ""
        valueLabel = ""
        if volumeNode:
          nameLabel = volumeNode.GetName()
          xyToIJK = layerLogic.GetXYToIJKTransform().GetMatrix()
          ijkFloat = xyToIJK.MultiplyPoint(xyz+(1,))[:3]
          ijk = []
          for element in ijkFloat:
            try:
              index = int(round(element))
            except ValueError:
              index = 0
            ijk.append(index)
          if self.__dwvNode != None:
            # get the vector of values at IJK
            dwvImage = self.__dwvNode.GetImageData()
            nComponents = self.__dwvNode.GetNumberOfGradients()
            values = ''
            for c in range(nComponents):
              values = values + str(dwvImage.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c))+' '
            self.__vcValue.setText(values)
