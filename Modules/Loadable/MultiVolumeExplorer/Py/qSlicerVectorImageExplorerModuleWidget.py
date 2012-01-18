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

    self.layout = self.parent.layout()

    # this flag is 1 if there is an update in progress
    self.__updating = 1

    # Reference to the logic
    self.__logic = slicer.modulelogic.vtkSlicerVectorImageExplorerLogic()
    print 'Logic is ',self.__logic

    if not parent:
      self.__logic = slicer.modulelogic.vtkVectorImageExplorerLogic()
      self.setup()
      self.parent.setMRMLScene( slicer.mrmlScene )
      # after setup, be ready for events
      self.__updating = 0

      self.parent.show()

    self.__dwvNode = None
    self.__vcNode = None

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
    layout = qt.QGridLayout()
    # layout = qt.QFormLayout()
    w.setLayout(layout)
    self.layout.addWidget(w)
    w.show()
    self.layout = layout
    
    label = qt.QLabel('Input container:')
    self.__vcSelector = slicer.qMRMLNodeComboBox()
    self.__vcSelector.nodeTypes = ['vtkMRMLVectorImageContainerNode']
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.__vcSelector.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)
    self.__vcSelector.connect('currentNodeChanged(vtkMRMLNode*)', self.onInputChanged)
    self.__vcSelector.addEnabled = 1

    ## self.layout.addRow(label, self.__vcSelector)
    self.layout.addWidget(label)
    self.layout.addWidget(self.__vcSelector)


    # TODO: initialize the slider based on the contents of the labels array
    # slider to scroll over metadata stored in the vector container being explored
    self.__mdSlider = ctk.ctkSliderWidget()
    #self.__mdSlider.setRange(0,10)
    #self.__mdSlider.setValue(5)

    label = qt.QLabel('Vector scroller:')
    ## self.layout.addRow(label, self.__mdSlider)
    self.layout.addWidget(label)
    self.layout.addWidget(self.__mdSlider)

    self.__mdSlider.connect('valueChanged(double)', self.onSliderChanged)

    label = qt.QLabel('Label for the element shown:')
    self.__mdValue = qt.QLabel()
    ## self.layout.addRow(label, self.__mdValue)
    self.layout.addWidget(label)
    self.layout.addWidget(self.__mdValue)

    label = qt.QLabel('Vector content:')
    self.__vcValue = qt.QLabel()
    ## self.layout.addRow(label, self.__vcValue)
    self.layout.addWidget(label)
    self.layout.addWidget(self.__vcValue)

    # initialize slice observers (from DataProbe.py)
    # keep list of pairs: [observee,tag] so they can be removed easily
    self.styleObserverTags = []
    # keep a map of interactor styles to sliceWidgets so we can easily get sliceLogic
    self.sliceWidgetsPerStyle = {}
    self.refreshObservers()

    # add chart container widget
    ##chartWidget = qt.QWidget()
    ##chartWidgetLayout = qt.QGridLayout()
    ##chartWidget.setLayout(chartWidgetLayout)
    ##self.__chartView = ctk.ctkVTKChartView(chartWidget)
    self.__chartView = ctk.ctkVTKChartView(w)
    ##self.layout.addRow(self.__chartView)
    self.layout.addWidget(self.__chartView)
    ##chartWidgetLayout.addWidget(self.__chartView)
    ##self.layout.addRow(chartWidget)
    ##chartWidget.show()

    self.__chart = self.__chartView.chart()
    self.__chartTable = vtk.vtkTable()
    self.__xArray = vtk.vtkFloatArray()
    self.__yArray = vtk.vtkFloatArray()
    # will crash if there is no name
    self.__xArray.SetName('X')
    self.__yArray.SetName('Y')
    self.__chartTable.AddColumn(self.__xArray)
    self.__chartTable.AddColumn(self.__yArray)
     
  def onSliderChanged(self, newValue):
    value = self.__mdSlider.value
    self.__mdValue.setText(str(newValue))

    if self.__dwvNode != None:
      dwvDisplayNode = self.__dwvNode.GetDisplayNode()
      dwvDisplayNode.SetDiffusionComponent(newValue)

  def onMRMLSceneChanged(self, mrmlScene):
    print 'onMRMLSceneChanged called'
    self.__vcSelector.setMRMLScene(slicer.mrmlScene)
    self.onInputChanged()
    '''
    if mrmlScene != self.__logic.GetMRMLScene():
      self.__logic.SetMRMLScene(mrmlScene)
      self.__logic.RegisterNodes()
      self.__logic.InitializeEventListeners()
    self.__logic.GetMRMLManager().SetMRMLScene(mrmlScene)
    '''
    
  def onInputChanged(self):
    print 'onInputChanged() called'
    self.__vcNode = self.__vcSelector.currentNode()
    if self.__vcNode != None:
       self.__dwvNode = self.__vcNode.GetDWVNode()
       print 'Active DWV node: ', self.__dwvNode
       if self.__dwvNode != None:
         self.__mdSlider.minimum = 0
         self.__mdSlider.maximum = self.__dwvNode.GetNumberOfGradients()-1
         self.__chartTable.SetNumberOfRows(self.__dwvNode.GetNumberOfGradients())
   
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
            extent = dwvImage.GetExtent()
            for c in range(nComponents):
              if ijk[0]>=0 and ijk[1]>=0 and ijk[2]>=0 and ijk[0]<extent[1] and ijk[1]<extent[3] and ijk[2]<extent[5]:
                val = dwvImage.GetScalarComponentAsDouble(ijk[0],ijk[1],ijk[2],c)
                values = values + str(val)+' '
                self.__chartTable.SetValue(c, 0, c)
                self.__chartTable.SetValue(c, 1, val)
              else:
                break
            
            if values != '':
              #self.__vcValue.setText(values)
              self.__chart.RemovePlot(0)
              self.__chart.RemovePlot(0)
              # self.__chart.GetAxis(0).SetTitle(self.__vcNode.GetLabel ???)
              plot = self.__chart.AddPlot(0)
              plot.SetInput(self.__chartTable, 0, 1)
              # seems to update only after another plot?..
              self.__chart.AddPlot(0)
