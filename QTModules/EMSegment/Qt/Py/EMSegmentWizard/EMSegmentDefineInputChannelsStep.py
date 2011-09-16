from __main__ import qt, ctk, tcl
import PythonQt

from EMSegmentStep import *
from Helper import *
from EMSegmentDynamicFrame import *

class EMSegmentDefineInputChannelsStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '2. Define Input Datasets' )
    self.setDescription( 'Name how many volumes should be segmented and select the set of scans for segmentation.' )

    self.__parent = super( EMSegmentDefineInputChannelsStep, self )

    self.__stepid = stepid
    self.__dynamicFrame = None
    self.__updating = 0

  def isSimpleMode( self ):
    '''
    '''
    if self.__stepid == ( str( Helper.GetNthStepId( 2 ) ) + str( 'Simple' ) ):
      # we are in simple mode
      return True
    else:
      return False

  def dynamicFrame( self ):
    '''
    '''
    if not self.__dynamicFrame:

      self.__dynamicFrame = EMSegmentDynamicFrame()

    return self.__dynamicFrame

  def createUserInterface( self ):
    '''
    '''

    # disable the next button in simple mode
    if self.isSimpleMode():
      self.workflow().goBackToOriginStepUponSuccess = False
      self.buttonBoxHints = self.NextButtonHidden
      slicer.modules.emsegmentsimplemode = True
      slicer.modules.emsegmentsimplestep2 = self
    else:
      slicer.modules.emsegmentsimplemode = False
      slicer.modules.emsegmentsimplestep2 = None

    self.__layout = self.__parent.createUserInterface()

    # the input channels
    inputChannelGroupBox = qt.QGroupBox()
    inputChannelGroupBox.setTitle( 'Input Datasets' )
    inputChannelGroupBox.toolTip = 'Please configure the datasets which should be segmented.'
    self.__layout.addWidget( inputChannelGroupBox )

    inputChannelGroupBoxLayout = qt.QFormLayout( inputChannelGroupBox )

    self.__inputChannelList = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentInputChannelListWidget()
    self.__inputChannelList.setMRMLManager( self.mrmlManager() )
    inputChannelGroupBoxLayout.addWidget( self.__inputChannelList )

    # add empty row
    self.__layout.addRow( "", qt.QWidget() )

    # registration settings
    input2inputChannelRegistration = qt.QGroupBox()
    input2inputChannelRegistration.setTitle( 'Input-to-Input Channel Registration' )
    self.__layout.addWidget( input2inputChannelRegistration )

    input2inputChannelRegistrationLayout = qt.QFormLayout( input2inputChannelRegistration )

    self.__alignInputScansCheckBox = qt.QCheckBox()
    self.__alignInputScansCheckBox.toolTip = 'Toggle to align the input datasets.'
    input2inputChannelRegistrationLayout.addRow( 'Align input datasets:', self.__alignInputScansCheckBox )
    self.__alignInputScansCheckBox.connect( "stateChanged(int)", self.propagateToMRML )

    # add empty row
    self.__layout.addRow( "", qt.QWidget() )

    if self.isSimpleMode():
      #
      # dynamic frame
      #
      dynamicFrame = qt.QGroupBox()
      dynamicFrame.setTitle( 'Check List' )
      dynamicFrame.toolTip = 'Please check anything applicable.'
      self.__layout.addWidget( dynamicFrame )
      dynamicFrameLayout = qt.QVBoxLayout( dynamicFrame )

      # .. now pass the layout to the dynamicFrame
      self.dynamicFrame().setLayout( dynamicFrameLayout )
      #
      # end of dynamic frame
      #


  def propagateToMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      self.__inputChannelList.updateMRMLFromWidget()

      self.mrmlManager().SetEnableTargetToTargetRegistration( int( self.__alignInputScansCheckBox.isChecked() ) )

      if self.isSimpleMode():
        # propagate dynamic frame settings to MRML
        self.dynamicFrame().SaveSettingsToMRML()

      self.__updating = 0



  def loadFromMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      self.__inputChannelList.updateWidgetFromMRML()

      if self.mrmlManager().GetEnableTargetToTargetRegistration() == 0:
        self.__alignInputScansCheckBox.setChecked( False )
      else:
        self.__alignInputScansCheckBox.setChecked( True )

      if self.isSimpleMode():
        # update the dynamic frame from MRML
        self.dynamicFrame().LoadSettingsFromMRML()

      self.__updating = 0



  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    if self.isSimpleMode():
      self.logic().SourceTaskFiles()

      # clear the dynamic panel
      self.dynamicFrame().setMRMLManager( self.mrmlManager() )
      self.dynamicFrame().clearElements()

      logicTclName = self.logic().GetSlicerCommonInterface().GetTclNameFromPointer( self.logic() )

      tcl( '::EMSegmenterSimpleTcl::ShowCheckList ' + str( logicTclName ) )

    self.loadFromMRML()



  def onExit( self, goingTo, transitionType ):
    '''
    '''
    self.propagateToMRML()

    if self.isSimpleMode():

      returnValue = tcl( "::EMSegmenterSimpleTcl::ValidateCheckList" )

      if int( returnValue ) != 0:
        # error
        messageBox = qt.QMessageBox.warning( self, "Error", "Could not validate the Checklist. Please double check your settings!" )
        return

      self.mrmlManager().GetWorkingDataNode().SetAlignedTargetNodeIsValid( 0 )
      self.mrmlManager().GetWorkingDataNode().SetAlignedAtlasNodeIsValid( 0 )

      # disable questions at the pre-processing step
      preProcessingStep = slicer.modules.emsegmentPreprocessingStep
      if preProcessingStep:
        preProcessingStep.disableQuestions()

    else:

      # disable questions at the pre-processing step
      preProcessingStep = slicer.modules.emsegmentPreprocessingStep
      if preProcessingStep:
        preProcessingStep.enableQuestions()


    self.__parent.onExit( goingTo, transitionType )



  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    # we need at least one input channel
    if self.__inputChannelList.inputChannelCount() == 0:
      self.__parent.validationFailed( desiredBranchId, 'Input Channel Error', 'Please add at least one input channel!' )
      return

    # we need an assigned volume for each channel
    for c in range( self.__inputChannelList.inputChannelCount() ):
      if not self.__inputChannelList.inputChannelVolume( c ):
        self.__parent.validationFailed( desiredBranchId, 'Input Channel Error', 'Please assign a volume to each input channel!' )
        return

    # check if all channels have different volumes assigned
    if self.__inputChannelList.identicalInputVolumes():
      self.__parent.validationFailed( desiredBranchId, 'Input Channel Error', 'Please assign different volumes to individual input channel!' )
      return

    # number of input channels changed
    if self.__inputChannelList.inputChannelCount() != self.mrmlManager().GetGlobalParametersNode().GetNumberOfTargetInputChannels():
      answer = qt.QMessageBox.question( self, "Change the number of input channels?", "Are you sure you want to change the number of input images?", qt.QMessageBox.Yes | qt.QMessageBox.No )
      if answer == qt.QMessageBox.No:
        self.__parent.validationFailed( desiredBranchId, '', '', False )
        return

    # check if all channels have different names
    if self.__inputChannelList.identicalInputChannelNames():
      self.__parent.validationFailed( desiredBranchId, 'Input Channel Error', 'Please assign different names to individual input channel!' )
      return

    self.__parent.validationSucceeded( desiredBranchId )


