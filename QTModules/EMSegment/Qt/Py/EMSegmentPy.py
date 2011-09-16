from __main__ import vtk, qt, ctk, slicer

import EMSegmentWizard
from EMSegmentWizard import Helper

class EMSegmentPy:
  def __init__( self, parent ):
    parent.title = "EMSegment"
    parent.category = "Segmentation"
    parent.contributor = "Daniel Haehn"
    parent.helpText = """<b>EMSegment Module:</b>  Segment a set of set of images (target images) using the tree-based EM segmentation algorithm<br><br>Use the pull down menu to select from a collection of tasks or create a new one.<br>Use the 'Back' and 'Next' to navigate through the stages of filling in the algorithm parameters.\n\nWhen all parameters are specified, use the 'segmentation' button. \n\nFor latest updates, new tasks, and detail help please visit <a>http://www.slicer.org/slicerWiki/index.php/Modules:EMSegmenter-3.6</a> <br><br> <b>The work was reported in:</b> <br>K.M. Pohl et. A hierarchical algorithm for MR brain image parcellation. IEEE Transactions on Medical Imaging, 26(9),pp 1201-1212, 2007.<br><br>Please restart 3D Slicer in order to choose a different task after a segmentation."""
    parent.acknowledgementText = """<img src=':/Icons/UPenn_logo.png'><br><br>This module is currently maintained by Daniel Haehn and Kilian Pohl (SBIA,UPenn). The work is currently supported by an ARRA supplement to NAC and the Slicer Community (see also <a>http://www.slicer.org</a>). <br><br>The work was reported in  <br>K.M. Pohl et. A hierarchical algorithm for MR brain image parcellation. IEEE Transactions on Medical Imaging, 26(9),pp 1201-1212, 2007."""
    parent.icon = qt.QIcon( ":/Icons/EMSegment.png" )
    self.parent = parent

class EMSegmentPyWidget:
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

    # register default slots
    #self.parent.connect('mrmlSceneChanged(vtkMRMLScene*)', self.onMRMLSceneChanged)      


  def logic( self ):
    if not self.__logic:
        self.__logic = slicer.modulelogic.vtkEMSegmentLogic()
        self.__logic.SetModuleName( "EMSegment" )
        self.__logic.SetMRMLScene( slicer.mrmlScene )
        self.__logic.RegisterNodes()
        self.__logic.InitializeEventListeners()

    return self.__logic

  def mrmlManager( self ):
    if not self.__mrmlManager:
        self.__mrmlManager = self.logic().GetMRMLManager()
        self.__mrmlManager.SetMRMLScene( slicer.mrmlScene )

    return self.__mrmlManager


  def setup( self ):
    '''
    Create and start the EMSegment workflow.
    '''
    self.workflow = ctk.ctkWorkflow()

    workflowWidget = ctk.ctkWorkflowStackedWidget()
    workflowWidget.setWorkflow( self.workflow )

    workflowWidget.buttonBoxWidget().nextButtonDefaultText = ""
    workflowWidget.buttonBoxWidget().backButtonDefaultText = ""

    # create all wizard steps
    selectTaskStep = EMSegmentWizard.EMSegmentSelectTaskStep( Helper.GetNthStepId( 1 ) )
    defineInputChannelsSimpleStep = EMSegmentWizard.EMSegmentDefineInputChannelsStep( Helper.GetNthStepId( 2 ) + 'Simple' ) # simple branch
    defineInputChannelsAdvancedStep = EMSegmentWizard.EMSegmentDefineInputChannelsStep( Helper.GetNthStepId( 2 ) + 'Advanced' ) # advanced branch
    defineAnatomicalTreeStep = EMSegmentWizard.EMSegmentDefineAnatomicalTreeStep( Helper.GetNthStepId( 3 ) )
    defineAtlasStep = EMSegmentWizard.EMSegmentDefineAtlasStep( Helper.GetNthStepId( 4 ) )
    editRegistrationParametersStep = EMSegmentWizard.EMSegmentEditRegistrationParametersStep( Helper.GetNthStepId( 5 ) )
    definePreprocessingStep = EMSegmentWizard.EMSegmentDefinePreprocessingStep( Helper.GetNthStepId( 6 ) )
    specifyIntensityDistributionStep = EMSegmentWizard.EMSegmentSpecifyIntensityDistributionStep( Helper.GetNthStepId( 7 ) )
    editNodeBasedParametersStep = EMSegmentWizard.EMSegmentEditNodeBasedParametersStep( Helper.GetNthStepId( 8 ) )
    miscStep = EMSegmentWizard.EMSegmentDefineMiscParametersStep( Helper.GetNthStepId( 9 ) )
    statisticsStep = EMSegmentWizard.EMSegmentLabelStatisticsStep( Helper.GetNthStepId( 11 ) )
    segmentStep = EMSegmentWizard.EMSegmentStartSegmentationStep( Helper.GetNthStepId( 10 ) )

    # add the wizard steps to an array for convenience
    allSteps = []

    allSteps.append( selectTaskStep )
    allSteps.append( defineInputChannelsSimpleStep )
    allSteps.append( defineInputChannelsAdvancedStep )
    allSteps.append( defineAnatomicalTreeStep )
    allSteps.append( defineAtlasStep )
    allSteps.append( editRegistrationParametersStep )
    allSteps.append( definePreprocessingStep )
    allSteps.append( specifyIntensityDistributionStep )
    allSteps.append( editNodeBasedParametersStep )
    allSteps.append( miscStep )
    allSteps.append( statisticsStep )
    allSteps.append( segmentStep )

    # Add transition for the first step which let's the user choose between simple and advanced mode
    self.workflow.addTransition( selectTaskStep, defineInputChannelsSimpleStep, 'SimpleMode' )
    self.workflow.addTransition( selectTaskStep, defineInputChannelsAdvancedStep, 'AdvancedMode' )

    # Add transitions associated to the simple mode
    self.workflow.addTransition( defineInputChannelsSimpleStep, defineAnatomicalTreeStep )

    # Add transitions associated to the advanced mode
    self.workflow.addTransition( defineInputChannelsAdvancedStep, defineAnatomicalTreeStep )

    # .. add transitions for the rest of the advanced mode steps
    for i in range( 3, len( allSteps ) - 1 ):
      self.workflow.addTransition( allSteps[i], allSteps[i + 1] )

    # Propagate the workflow, the logic and the MRML Manager to the steps
    for s in allSteps:
        s.setWorkflow( self.workflow )
        s.setLogic( self.logic() )
        s.setMRMLManager( self.mrmlManager() )

    # disable the error text which showed up when jumping to the (invisible) segment step
    workflowWidget.workflowGroupBox().errorTextEnabled = False
    self.workflow.goBackToOriginStepUponSuccess = False

    # start the workflow and show the widget
    self.workflow.start()
    workflowWidget.visible = True
    self.layout.addWidget( workflowWidget )

    # enable global access to the dynamicFrames on step 2 and step 6
    slicer.modules.emsegmentLogic = self.logic()
    slicer.modules.emsegmentSimpleDynamicFrame = defineInputChannelsSimpleStep.dynamicFrame()
    slicer.modules.emsegmentPreprocessingStep = definePreprocessingStep

    # compress the layout
    #self.layout.addStretch(1)        


  def GetDynamicFrame( self ):
    '''
    '''
    return EMSegmentWizard.EMSegmentDynamicFrame()
