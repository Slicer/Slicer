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

    self.__mdValue = qt.QLabel()
    self.layout.addRow(self.__mdValue)

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
