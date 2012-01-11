from __main__ import vtk, qt, ctk, slicer

from EMSegmentWizard import Helper

class qSlicerVectorImageExplorerModuleWidget:
  def __init__( self, parent=None ):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout( qt.QVBoxLayout() )
    else:
      self.parent = parent

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


  def setup( self ):
    '''
    Create and start the EMSegment workflow.
    '''

    # Use the logic associated with the module
    if not self.__logic:
      self.__logic = self.parent.module().logic()

    self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)

    workflow = ctk.ctkWorkflow()

    workflowWidget = ctk.ctkWorkflowStackedWidget()
    workflowWidget.setWorkflow( workflow )

    workflowWidget.visible = True

    self.layout.addWidget( workflowWidget )

    # Keep track of workflow and workflowWidget references
    self.__workflow = workflow
    self.__workflowWidget = workflowWidget

  def onMRMLSceneChanged(self, mrmlScene):
    return
    '''
    if mrmlScene != self.__logic.GetMRMLScene():
      self.__logic.SetMRMLScene(mrmlScene)
      self.__logic.RegisterNodes()
      self.__logic.InitializeEventListeners()
    self.__logic.GetMRMLManager().SetMRMLScene(mrmlScene)
    '''
