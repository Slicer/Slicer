from __main__ import qt, ctk, vtk
import PythonQt
import os
import math

from EMSegmentStep import *
from Helper import *

class EMSegmentDefineMiscParametersStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '9. Define Miscellaneous Parameters' )
    self.setDescription( 'Define miscellaneous parameters before performing segmentation' )

    self.__parent = super( EMSegmentDefineMiscParametersStep, self )

    self.__saveButton = None
    self.__saveIntermediateResultsCheckBox = None
    self.__selectDirectoryButton = None
    self.__subparcellationCheckBox = None
    self.__minimumIslandSizeSpinBox = None
    self.__twoDIslandNeighborhoodCheckBox = None
    self.__multithreadingCheckBox = None

    self.__updating = 0;
    self.__directory = None
    self.__templateFile = None
    self.__roi = None


  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    # deactivate next button since it is the last step
    self.buttonBoxHints = self.NextButtonDisabled

    # the ROI parameters
    voiGroupBox = qt.QGroupBox()
    voiGroupBox.setTitle( 'Define VOI' )
    self.__layout.addWidget( voiGroupBox )

    voiGroupBoxLayout = qt.QFormLayout( voiGroupBox )

    self.__roiWidget = PythonQt.qSlicerAnnotationsModuleWidgets.qMRMLAnnotationROIWidget()
    self.__roiWidget.toolTip = 'Select a sub-volume for segmentation. Then, only the selected area will be segmented. By default, the complete volume will be segmented.'
    voiGroupBoxLayout.addWidget( self.__roiWidget )
    self.__roi = slicer.vtkMRMLAnnotationROINode()
    self.__roi.SetXYZ( [0, 0, 0] );
    self.__roi.SetRadiusXYZ( 100, 100, 100 )
    self.__roi.Initialize( slicer.mrmlScene )
    self.__roi.SetVisibility( 0 )
    self.__roi.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateMRMLFromROI )
    self.__roiWidget.setMRMLAnnotationROINode( self.__roi )
    self.__roiWidget.setDisplayClippingBox( 0 )

    #
    # save groupbox
    #
    saveGroupBox = qt.QGroupBox()
    saveGroupBox.setTitle( 'Save' )
    self.__layout.addWidget( saveGroupBox )

    saveGroupBoxLayout = qt.QFormLayout ( saveGroupBox )

    self.__saveButton = qt.QPushButton( 'Create' )
    self.__saveButton.toolTip = 'Create a template file which can be shared and used to re-apply the configured task.'
    saveGroupBoxLayout.addRow( "Create Template File:", self.__saveButton )
    self.__saveButton.connect( 'clicked()', self.onSaveButtonClicked )

    self.__saveIntermediateResultsCheckBox = qt.QCheckBox()
    self.__saveIntermediateResultsCheckBox.toolTip = 'Toggle to save intermediate results as well.'
    saveGroupBoxLayout.addRow( "Save Intermediate Results:", self.__saveIntermediateResultsCheckBox )
    self.__saveIntermediateResultsCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__selectDirectoryButton = qt.QPushButton( 'Select' )
    self.__selectDirectoryButton.toolTip = 'Select a directory to save intermediate results, if selected.'
    saveGroupBoxLayout.addRow( "Select Intermediate Directory:", self.__selectDirectoryButton )
    self.__selectDirectoryButton.connect( 'clicked()', self.onSelectDirectoryButtonClicked )

    #
    # post processing groupbox
    #
    postProcessingGroupBox = qt.QGroupBox()
    postProcessingGroupBox.setTitle( 'Postprocessing' )
    self.__layout.addWidget( postProcessingGroupBox )

    postProcessingGroupBoxLayout = qt.QFormLayout( postProcessingGroupBox )

    self.__subparcellationCheckBox = qt.QCheckBox()
    self.__subparcellationCheckBox.toolTip = 'Enable subpacellation.'
    postProcessingGroupBoxLayout.addRow( "Subparcellation enabled:", self.__subparcellationCheckBox )
    self.__subparcellationCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__minimumIslandSizeSpinBox = qt.QSpinBox()
    self.__minimumIslandSizeSpinBox.toolTip = 'Configure the minimum island size.'
    self.__minimumIslandSizeSpinBox.minimum = 0
    self.__minimumIslandSizeSpinBox.maximum = 200
    self.__minimumIslandSizeSpinBox.singleStep = 1
    postProcessingGroupBoxLayout.addRow( "Minimum island size:", self.__minimumIslandSizeSpinBox )
    self.__minimumIslandSizeSpinBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__twoDIslandNeighborhoodCheckBox = qt.QCheckBox()
    self.__twoDIslandNeighborhoodCheckBox.toolTip = 'Enable 2D Island Neighborhood'
    postProcessingGroupBoxLayout.addRow( "2D Island Neighborhood:", self.__twoDIslandNeighborhoodCheckBox )
    self.__twoDIslandNeighborhoodCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    #
    # Misc. groupbox
    #
    miscGroupBox = qt.QGroupBox()
    miscGroupBox.setTitle( 'Misc.' )
    self.__layout.addWidget( miscGroupBox )

    miscGroupBoxLayout = qt.QFormLayout( miscGroupBox )

    self.__multithreadingCheckBox = qt.QCheckBox()
    self.__multithreadingCheckBox.toolTip = 'Enhance speed by using multiple threads.'
    miscGroupBoxLayout.addRow( "Multi-threading enabled:", self.__multithreadingCheckBox )
    self.__multithreadingCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

  def onSelectDirectoryButtonClicked( self ):
    '''
    '''
    dialog = qt.QFileDialog()
    dir = dialog.getExistingDirectory()
    if dir:
      self.__directory = dir
      self.__selectDirectoryButton.text = os.path.split( dir )[-1]
      self.propagateToMRML()
    else:
      self.__directory = None


  def onSaveButtonClicked( self ):
    '''
    '''
    dialog = qt.QFileDialog()
    file = dialog.getSaveFileName()
    if file:
      self.__templateFile = file
      self.__saveButton.text = os.path.split( file )[-1]
      self.saveTemplate()
    else:
      self.__templateFile = None


  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    self.loadFromMRML()


  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )

  def saveTemplate( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      if self.__templateFile:
        self.mrmlManager().SetSaveTemplateFilename( self.__templateFile )

        ret = self.mrmlManager().CreateTemplateFile()
        if ret != 0:
          messageBox = qt.QMessageBox.warning( self, 'Create Template Error', 'Could not create template - the filename of the template was probably not in the same directory as the scene' )

      else:
        self.mrmlManager().SetSaveTemplateFilename( None )

      self.__updating = 0


  def propagateToMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      if self.__saveIntermediateResultsCheckBox:
        self.mrmlManager().SetSaveIntermediateResults( int( self.__saveIntermediateResultsCheckBox.checked ) )

      if self.__directory and os.path.isdir( self.__directory ) and self.__saveIntermediateResultsCheckBox.checked:
        self.mrmlManager().SetSaveWorkingDirectory( self.__directory )
      else:
        self.mrmlManager().SetSaveWorkingDirectory( None )

      if self.__subparcellationCheckBox:
        self.mrmlManager().SetEnableSubParcellation( int( self.__subparcellationCheckBox.checked ) )
      if self.__minimumIslandSizeSpinBox:
        self.mrmlManager().SetMinimumIslandSize( int( self.__minimumIslandSizeSpinBox.value ) )
      if self.__twoDIslandNeighborhoodCheckBox:
        self.mrmlManager().SetIsland2DFlag( int( self.__twoDIslandNeighborhoodCheckBox.checked ) )

      if self.__multithreadingCheckBox:
        self.mrmlManager().SetEnableMultithreading( int( self.__multithreadingCheckBox.checked ) )

      self.__updating = 0


  def loadFromMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      if self.__roiWidget:
        self.updateROIFromMRML()

      if self.__saveButton:
        saveTemplateFilename = self.mrmlManager().GetSaveTemplateFilename()
        if saveTemplateFilename:
          self.__saveButton.text = os.path.split( saveTemplateFilename )[-1]
        else:
          self.__saveButton.text = 'Create'

      if self.__saveIntermediateResultsCheckBox:
        self.__saveIntermediateResultsCheckBox.setChecked( self.mrmlManager().GetSaveIntermediateResults() )

      if self.__selectDirectoryButton:
        workingDirectory = self.mrmlManager().GetSaveWorkingDirectory()
        if workingDirectory and os.path.isdir( workingDirectory ):
          self.__selectDirectoryButton.text = os.path.split( workingDirectory )[-1]
        else:
          self.__selectDirectoryButton.text = 'Select'

      if self.__subparcellationCheckBox:
        self.__subparcellationCheckBox.setChecked( self.mrmlManager().GetEnableSubParcellation() )
      if self.__minimumIslandSizeSpinBox:
        self.__minimumIslandSizeSpinBox.value = self.mrmlManager().GetMinimumIslandSize()
      if self.__twoDIslandNeighborhoodCheckBox:
        self.__twoDIslandNeighborhoodCheckBox.setChecked( self.mrmlManager().GetIsland2DFlag() )

      if self.__multithreadingCheckBox:
        self.__multithreadingCheckBox.setChecked( self.mrmlManager().GetEnableMultithreading() )

      self.__updating = 0


  def updateROIFromMRML( self ):
    '''
    '''
    if not self.__roi:
      return

    volumeNode = self.mrmlManager().GetWorkingDataNode().GetInputTargetNode().GetNthVolumeNode( 0 )

    if not volumeNode:
      return

    self.logic().DefineValidSegmentationBoundary()

    minPoint = [0, 0, 0]
    maxPoint = [0, 0, 0]
    self.mrmlManager().GetSegmentationBoundaryMin( minPoint )
    self.mrmlManager().GetSegmentationBoundaryMax( maxPoint )

    roiMinIJK = [0, 0, 0, 0]
    roiMaxIJK = [0, 0, 0, 0]
    roiMinRAS = [0, 0, 0, 0]
    roiMaxRAS = [0, 0, 0, 0]
    radius = [0, 0, 0]
    center = [0, 0, 0]

    roiMinIJK[0] = minPoint[0] - 1
    roiMinIJK[1] = minPoint[1] - 1
    roiMinIJK[2] = minPoint[2] - 1
    roiMinIJK[3] = 1

    roiMaxIJK[0] = maxPoint[0] - 1
    roiMaxIJK[1] = maxPoint[1] - 1
    roiMaxIJK[2] = maxPoint[2] - 1
    roiMaxIJK[3] = 1

    ijkToRas = vtk.vtkMatrix4x4()
    volumeNode.GetIJKToRASMatrix( ijkToRas )
    ijkToRas.MultiplyPoint( roiMinIJK, roiMinRAS )
    ijkToRas.MultiplyPoint( roiMaxIJK, roiMaxRAS )

    center[0] = ( roiMaxRAS[0] + roiMinRAS[0] ) / 2.0
    center[1] = ( roiMaxRAS[1] + roiMinRAS[1] ) / 2.0
    center[2] = ( roiMaxRAS[2] + roiMinRAS[2] ) / 2.0

    radius[0] = math.fabs( roiMaxRAS[0] - roiMinRAS[0] ) / 2.0
    radius[1] = math.fabs( roiMaxRAS[1] - roiMinRAS[1] ) / 2.0
    radius[2] = math.fabs( roiMaxRAS[2] - roiMinRAS[2] ) / 2.0

    self.__roi.SetXYZ( center[0], center[1], center[2] )
    self.__roi.SetRadiusXYZ( radius[0], radius[1], radius[2] )
    self.__roi.Modified()


  def updateMRMLFromROI( self, interactorStyleTrackballCamera, event ):
    '''
    '''
    if not self.__roi:
      return

    volumeNode = self.mrmlManager().GetWorkingDataNode().GetInputTargetNode().GetNthVolumeNode( 0 )

    if not volumeNode:
      return

    center = [0, 0, 0]
    self.__roi.GetXYZ( center )
    radius = [0, 0, 0]
    self.__roi.GetRadiusXYZ( radius )

    box0IJK = [0, 0, 0, 0]
    box0RAS = [0, 0, 0, 0]
    box1IJK = [0, 0, 0, 0]
    box1RAS = [0, 0, 0, 0]

    box0RAS[0] = center[0] - radius[0]
    box0RAS[1] = center[1] - radius[1]
    box0RAS[2] = center[2] - radius[2]
    box0RAS[3] = 1

    box1RAS[0] = center[0] + radius[0]
    box1RAS[1] = center[1] + radius[1]
    box1RAS[2] = center[2] + radius[2]
    box1RAS[3] = 1

    rasToIJK = vtk.vtkMatrix4x4()
    volumeNode.GetRASToIJKMatrix( rasToIJK )
    rasToIJK.MultiplyPoint( box0RAS, box0IJK )
    rasToIJK.MultiplyPoint( box1RAS, box1IJK )

    for i in range( 3 ):
      if box0IJK[i] > box1IJK[i]:
        tmp = box0IJK[i]
        box0IJK[i] = box1IJK[i]
        box1IJK[i] = tmp

    pointMin = [int( box0IJK[0] + 1.0 ), int( box0IJK[1] + 1.0 ), int( box0IJK[2] + 1.0 )]
    pointMax = [int( box1IJK[0] + 1.0 ), int( box1IJK[1] + 1.0 ), int( box1IJK[2] + 1.0 )]

    self.mrmlManager().SetSegmentationBoundaryMin( pointMin )
    self.mrmlManager().SetSegmentationBoundaryMax( pointMax )

