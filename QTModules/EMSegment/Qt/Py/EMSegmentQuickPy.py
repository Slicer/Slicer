from __main__ import vtk, qt, ctk, slicer

import EMSegmentWizard
from EMSegmentWizard import Helper

class EMSegmentQuickPy:
  def __init__( self, parent ):
    parent.title = "EMSegment Easy"
    parent.category = "Segmentation"
    parent.contributor = "Daniel Haehn"
    parent.helpText = """<b>EMSegment Quick:</b> This module provides EM segmentation without an atlas.\nIt is possible to segment different structures by manual sampling. """
    parent.acknowledgementText = """<img src=':/Icons/UPenn_logo.png'><br><br>This module was developed by Daniel Haehn and Kilian Pohl (SBIA, UPenn). The work is currently supported by an ARRA supplement to NAC and the Slicer Community (see also <a>http://www.slicer.org</a>). <br><br>The work was reported in  <br>K.M. Pohl et. A hierarchical algorithm for MR brain image parcellation. IEEE Transactions on Medical Imaging, 26(9),pp 1201-1212, 2007."""
    parent.icon = qt.QIcon( ":/Icons/EMSegmentQuick.png" )
    self.parent = parent

class EMSegmentQuickPyWidget:
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

      try:
        # try to grab the logic from the original emsegmenter first        
        self.__logic = slicer.modules.emsegmentlogic
      except AttributeError:
        self.__logic = slicer.modulelogic.vtkEMSegmentLogic()
        self.__logic.SetModuleName( "EMSegment Quick" )
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
    step1 = EMSegmentWizard.EMSegmentQuickStep1( Helper.GetNthStepId( 2 ) )
    step2 = EMSegmentWizard.EMSegmentQuickStep2( Helper.GetNthStepId( 3 ) )
    step3 = EMSegmentWizard.EMSegmentQuickStep3( Helper.GetNthStepId( 7 ) )
    step4 = EMSegmentWizard.EMSegmentQuickStep4( Helper.GetNthStepId( 9 ) )
    step5 = EMSegmentWizard.EMSegmentQuickStep5( Helper.GetNthStepId( 11 ) )
    segmentStep = EMSegmentWizard.EMSegmentQuickSegmentationStep( Helper.GetNthStepId( 10 ) )

    # add the wizard steps to an array for convenience
    allSteps = []

    allSteps.append( step1 )
    allSteps.append( step2 )
    allSteps.append( step3 )
    allSteps.append( step4 )
    allSteps.append( step5 )
    allSteps.append( segmentStep )

    # .. add transitions for the rest of the advanced mode steps
    for i in range( 0, len( allSteps ) - 1 ):
      self.workflow.addTransition( allSteps[i], allSteps[i + 1] )

    # Propagate the workflow, the logic and the MRML Manager to the steps
    for s in allSteps:
        s.setWorkflow( self.workflow )
        s.setLogic( self.logic() )
        s.setMRMLManager( self.mrmlManager() )

    # disable the error text which showed up when jumping to the (invisible) segment step
    workflowWidget.workflowGroupBox().errorTextEnabled = False
    self.workflow.goBackToOriginStepUponSuccess = False

    slicer.modules.emsegmenteasystep1 = step1
    slicer.modules.emsegmenteasystep2 = step2

    # start the workflow and show the widget
    self.workflow.start()
    workflowWidget.visible = True
    self.layout.addWidget( workflowWidget )



    # compress the layout
    #self.layout.addStretch(1)        


  def GetDynamicFrame( self ):
    '''
    '''
    return EMSegmentWizard.EMSegmentDynamicFrame()
