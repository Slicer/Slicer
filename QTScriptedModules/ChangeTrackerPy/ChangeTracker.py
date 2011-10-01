from __main__ import vtk, qt, ctk, slicer

import ChangeTrackerWizard

class ChangeTracker:
  def __init__( self, parent ):
    parent.title = "ChangeTracker"
    parent.category = "Wizards"
    parent.contributor = "Andrey Fedorov, Ron Kikinis (PI)"
    parent.helpText = "THIS MODULE IS UNDER DEVELOPMENT!!!"
    parent.acknowledgementText = "UNDER DEVELOPMENT"
    self.parent = parent

class ChangeTrackerWidget:
  def __init__( self, parent=None ):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout( qt.QVBoxLayout() )
      self.parent.setMRMLScene( slicer.mrmlScene )
    else:
      self.parent = parent
    self.layout = self.parent.layout()

    # this flag is 1 if there is an update in progress
    self.__updating = 1

    # the pointer to the logic and the mrmlManager
    self.__mrmlManager = None
    self.__logic = None

    if not parent:
      self.setup()

      # after setup, be ready for events
      self.__updating = 0

      self.parent.show()

    if slicer.mrmlScene.GetTagByClassName( "vtkMRMLScriptedModuleNode" ) != 'ScriptedModule':
      slicer.mrmlScene.RegisterNodeClass(vtkMRMLScriptedModuleNode())

    # register default slots
    #self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)      


  #def logic( self ):
    #if not self.__logic:
    #    self.__logic = slicer.modulelogic.vtkChangeTrackerLogic()
    #    self.__logic.SetModuleName( "ChangeTracker" )
    #    self.__logic.SetMRMLScene( slicer.mrmlScene )
    #    self.__logic.RegisterNodes()
    #    self.__logic.InitializeEventListeners()

    #return self.__logic

  #def mrmlManager( self ):
  #  if not self.__mrmlManager:
  #      self.__mrmlManager = self.logic().GetMRMLManager()
  #      self.__mrmlManager.SetMRMLScene( slicer.mrmlScene )
  #
  #  return self.__mrmlManager


  def setup( self ):
    '''
    Create and start the ChangeTracker workflow.
    '''
    self.workflow = ctk.ctkWorkflow()

    workflowWidget = ctk.ctkWorkflowStackedWidget()
    workflowWidget.setWorkflow( self.workflow )

    workflowWidget.buttonBoxWidget().nextButtonDefaultText = ""
    workflowWidget.buttonBoxWidget().backButtonDefaultText = ""
    
    # create all wizard steps
    selectScansStep = ChangeTrackerWizard.ChangeTrackerSelectScansStep( 'SelectScans'  )
    defineROIStep = ChangeTrackerWizard.ChangeTrackerDefineROIStep( 'DefineROI'  )
    segmentROIStep = ChangeTrackerWizard.ChangeTrackerSegmentROIStep( 'SegmentROI'  )
    analyzeROIStep = ChangeTrackerWizard.ChangeTrackerAnalyzeROIStep( 'AnalyzeROI'  )
    reportROIStep = ChangeTrackerWizard.ChangeTrackerReportROIStep( 'ReportROI'  )

    # add the wizard steps to an array for convenience
    allSteps = []

    allSteps.append( selectScansStep )
    allSteps.append( defineROIStep )
    allSteps.append( segmentROIStep )
    allSteps.append( analyzeROIStep )
    allSteps.append( reportROIStep )

    # Add transition for the first step which let's the user choose between simple and advanced mode
    self.workflow.addTransition( selectScansStep, defineROIStep )
    self.workflow.addTransition( defineROIStep, segmentROIStep )
    self.workflow.addTransition( segmentROIStep, analyzeROIStep )
    self.workflow.addTransition( analyzeROIStep, reportROIStep )

    nNodes = slicer.mrmlScene.GetNumberOfNodesByClass('vtkMRMLScriptedModuleNode')

    self.parameterNode = None
    for n in xrange(nNodes):
      compNode = slicer.mrmlScene.GetNthNodeByClass(n, 'vtkMRMLScriptedModuleNode')
      nodeid = None
      if compNode.GetModuleName() == 'ChangeTracker':
        self.parameterNode = compNode
        print 'Found existing ChangeTracker parameter node'
        break
    if self.parameterNode == None:
      self.parameterNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLScriptedModuleNode')
      self.parameterNode.SetModuleName('ChangeTracker')
      slicer.mrmlScene.AddNode(self.parameterNode)
 
    # Propagate the workflow, the logic and the MRML Manager to the steps
    for s in allSteps:
        s.setWorkflow( self.workflow )
        s.setParameterNode (self.parameterNode)
    '''
        s.setLogic( self.logic() )
        s.setMRMLManager( self.mrmlManager() )
    '''

    # restore workflow step
    currentStep = self.parameterNode.GetParameter('currentStep')
    if currentStep != '':
      print 'Restoring workflow step to ', currentStep
      if currentStep == 'SelectScans':
        self.workflow.setInitialStep(selectScansStep)
      if currentStep == 'DefineROI':
        self.workflow.setInitialStep(defineROIStep)
      if currentStep == 'SegmentROI':
        self.workflow.setInitialStep(segmentROIStep)
      if currentStep == 'AnalyzeROI':
        self.workflow.setInitialStep(analyzeROIStep)
      if currentStep == 'ReportROI':
        self.workflow.setInitialStep(reportROIStep)
    else:
      print 'currentStep in parameter node is empty!'
        
    # start the workflow and show the widget
    self.workflow.start()
    workflowWidget.visible = True
    self.layout.addWidget( workflowWidget )

    # enable global access to the dynamicFrames on step 2 and step 6
    #slicer.modules.emsegmentSimpleDynamicFrame = defineInputChannelsSimpleStep.dynamicFrame()
    #slicer.modules.emsegmentAdvancedDynamicFrame = definePreprocessingStep.dynamicFrame()

    # compress the layout
      #self.layout.addStretch(1)        
 
  def enter(self):
    print "ChangeTracker: enter() called"
