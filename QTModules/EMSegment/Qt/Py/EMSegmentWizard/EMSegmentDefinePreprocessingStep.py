from __main__ import qt, ctk, tcl
import PythonQt

from EMSegmentStep import *
from Helper import *
from EMSegmentDynamicFrame import *

class EMSegmentDefinePreprocessingStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '6. Define Preprocessing' )
    self.setDescription( 'Answer questions for preprocessing of input images' )

    self.__parent = super( EMSegmentDefinePreprocessingStep, self )

    self.__stepid = stepid
    self.__dynamicFrame = None
    self.__askQuestionsBeforeRunningPreprocessing = True
    self.__performCalculation = False
    self.__updating = 0

  def disableQuestions( self ):
    '''
    '''
    self.__askQuestionsBeforeRunningPreprocessing = False
    self.__performCalculation = True

  def enableQuestions( self ):
    '''
    '''
    self.__askQuestionsBeforeRunningPreprocessing = True

  def dynamicFrame( self ):
    '''
    '''
    if not self.__dynamicFrame:

      self.__dynamicFrame = EMSegmentDynamicFrame()

    return self.__dynamicFrame

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

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

      self.__updating = 0



  def loadFromMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      # update the dynamic frame from MRML
      self.dynamicFrame().LoadSettingsFromMRML()

      self.__updating = 0



  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    slicer.modules.emsegmentAdvancedDynamicFrame = self.dynamicFrame()

    # we are always in advanced mode, so let's fill the dynamic frame
    self.logic().SourceTaskFiles()
    self.logic().SourcePreprocessingTclFiles()

    # clear the dynamic panel
    self.dynamicFrame().setMRMLManager( self.mrmlManager() )
    self.dynamicFrame().clearElements()

    logicTclName = self.logic().GetSlicerCommonInterface().GetTclNameFromPointer( self.logic() )

    tcl( '::EMSegmenterPreProcessingTcl::ShowUserInterface ' + str( logicTclName ) )

    self.loadFromMRML()


  def onExit( self, goingTo, transitionType ):
    '''
    '''

    # always save dynamic frame to mrml
    self.dynamicFrame().SaveSettingsToMRML()

    # only perform the following if we go forward
    if goingTo.id() != Helper.GetNthStepId( 5 ):
      # all ok, we are going forward..

      # check if we should perform the calculation
      if self.__performCalculation:
        self.runPreProcessing()

    self.__parent.onExit( goingTo, transitionType )


  def runPreProcessing( self ):
    '''
    '''
    # notify user
    message = qt.QMessageBox( qt.QMessageBox.NoIcon, "Please wait", "Please wait while pre-processing runs..", qt.QMessageBox.Ignore )
    message.setModal( False )
    message.show()

    # run preprocessing
    returnValue = tcl( "::EMSegmenterPreProcessingTcl::Run" )

    message.hide()

    if not returnValue or int( returnValue ) != 0:
      # something went wrong!
      # error message!
      messageBox = qt.QMessageBox.warning( self, "Error", "Pre-processing did not execute correctly!" )
      return

    workingDataNode = self.mrmlManager().GetWorkingDataNode()

    if workingDataNode:
      # set flags in the mrml nodes
      workingDataNode.SetAlignedTargetNodeIsValid( 1 )
      workingDataNode.SetAlignedAtlasNodeIsValid( 1 )

      # show preprocessing output in sliceViews
      volumeCollection = workingDataNode.GetInputTargetNode()
      if volumeCollection:
        outputNode = volumeCollection.GetNthVolumeNode( 0 )
        # propagate to sliceViews
        selectionNode = slicer.app.mrmlApplicationLogic().GetSelectionNode()
        selectionNode.SetReferenceActiveVolumeID( outputNode.GetID() )
        #selectionNode.SetReferenceSecondaryVolumeID( outputNode.GetID() )
        slicer.app.mrmlApplicationLogic().PropagateVolumeSelection()

    Helper.Info( '=============================================' )
    Helper.Info( 'Pre-processing completed successfully' )
    Helper.Info( '=============================================' )


  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    # check if CMTK should be used,
    # then try to find it
    # if this fails, ask if we should use BRAINSFit
    if self.mrmlManager().GetRegistrationPackageType() == self.mrmlManager().GetPackageTypeFromString( 'CMTK' ):
      # cmtk was selected
      cmtkPath = tcl( "::EMSegmenterPreProcessingTcl::Get_CMTK_Installation_Path" )

      if cmtkPath == "":
        # cmtk was not found, ask if we want BRAINSFit instead
        answer = qt.QMessageBox.question( self, "CMTK is not installed", "CMTK is not installed. Do you want to proceed with BRAINSTools instead?", qt.QMessageBox.Yes | qt.QMessageBox.No )
        if answer == qt.QMessageBox.No:
          # if no, exit here and stay at this step
          self.__parent.validationFailed( desiredBranchId, '', '', False )
          return

    # same for plastimatch
    if self.mrmlManager().GetRegistrationPackageType() == self.mrmlManager().GetPackageTypeFromString( 'PLASTIMATCH' ):
      # plastimatch was selected
      plastimatchPath = tcl( "::EMSegmenterPreProcessingTcl::Get_PLASTIMATCH_Installation_Path" )

      if plastimatchPath == "":
        # plastimatch was not found, ask if we want BRAINSFit instead
        answer = qt.QMessageBox.question( self, "PLASTIMATCH is not installed", "PLASTIMATCH is not installed. Do you want to proceed with BRAINSTools instead?", qt.QMessageBox.Yes | qt.QMessageBox.No )
        if answer == qt.QMessageBox.No:
          # if no, exit here and stay at this step
          self.__parent.validationFailed( desiredBranchId, '', '', False )
          return

    # same for DEMONS
    if self.mrmlManager().GetRegistrationPackageType() == self.mrmlManager().GetPackageTypeFromString( 'DEMONS' ):
      # DEMONS was selected
      demonsPath = tcl( "::EMSegmenterPreProcessingTcl::Get_DEMONS_Installation_Path" )

      if demonsPath == "":
        # demons was not found, ask if we want BRAINSFit instead
        answer = qt.QMessageBox.question( self, "DEMONS is not installed", "DEMONS is not installed. Do you want to proceed with BRAINSTools instead?", qt.QMessageBox.Yes | qt.QMessageBox.No )
        if answer == qt.QMessageBox.No:
          # if no, exit here and stay at this step
          self.__parent.validationFailed( desiredBranchId, '', '', False )
          return

    # same for DRAMMS
    if self.mrmlManager().GetRegistrationPackageType() == self.mrmlManager().GetPackageTypeFromString( 'DRAMMS' ):
      # dramms was selected
      drammsPath = tcl( "::EMSegmenterPreProcessingTcl::Get_DRAMMS_Installation_Path" )

      if drammsPath == "":
        # dramms was not found, ask if we want BRAINSFit instead
        answer = qt.QMessageBox.question( self, "DRAMMS is not installed", "DRAMMS is not installed. Do you want to proceed with BRAINSTools instead?", qt.QMessageBox.Yes | qt.QMessageBox.No )
        if answer == qt.QMessageBox.No:
          # if no, exit here and stay at this step
          self.__parent.validationFailed( desiredBranchId, '', '', False )
          return

    # same for ANTS
    if self.mrmlManager().GetRegistrationPackageType() == self.mrmlManager().GetPackageTypeFromString( 'ANTS' ):
      # ANTS was selected
      antsPath = tcl( "::EMSegmenterPreProcessingTcl::Get_ANTS_Installation_Path" )

      if antsPath == "":
        # ANTS was not found, ask if we want BRAINSFit instead
        answer = qt.QMessageBox.question( self, "ANTS is not installed", "ANTS is not installed. Do you want to proceed with BRAINSTools instead?", qt.QMessageBox.Yes | qt.QMessageBox.No )
        if answer == qt.QMessageBox.No:
          # if no, exit here and stay at this step
          self.__parent.validationFailed( desiredBranchId, '', '', False )
          return

    #
    # check if we should ask questions before running preprocessing and if yes do so!
    #
    if not self.__askQuestionsBeforeRunningPreprocessing:
      # no questions, just run it
      self.__parent.validationSucceeded( desiredBranchId )
      return

    # check if preprocessing already ran
    # if yes, ask for redo
    # if no, ask for start (which takes some time)
    workingDataNode = self.mrmlManager().GetWorkingDataNode()
    preprocessingAlreadyDone = ( workingDataNode.GetAlignedTargetNodeIsValid() and workingDataNode.GetAlignedAtlasNodeIsValid() )

    if preprocessingAlreadyDone:
      # ask if we should do it again
      answer = qt.QMessageBox.question( self, "Redo Pre-processing of images?", "Do you want to redo pre-processing of input images?", qt.QMessageBox.Yes | qt.QMessageBox.No )
      if answer == qt.QMessageBox.No:
        # no calculation
        self.__performCalculation = False
      else:
        # calculation
        self.__performCalculation = True

    else:
      # ask if we should run preprocessing
      # if not, stay at this step
      answer = qt.QMessageBox.question( self, "Start Pre-processing of images?", "Pre-processing of images might take a while. Do you want to proceed?", qt.QMessageBox.Yes | qt.QMessageBox.No )
      if answer == qt.QMessageBox.No:
        # if no, exit here and stay at this step
        self.__parent.validationFailed( desiredBranchId, '', '', False )
        return
      else:
        # perform the calculation!
        self.__performCalculation = True


    self.__parent.validationSucceeded( desiredBranchId )


