from __main__ import qt, ctk
import PythonQt

from EMSegmentStep import *
from Helper import *

class EMSegmentEditRegistrationParametersStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '5. Edit Registration Parameters' )
    self.setDescription( 'Specify atlas-to-dataset registration parameters.' )

    self.__parent = super( EMSegmentEditRegistrationParametersStep, self )
    self.__layout = None
    self.__channelComboBoxList = []
    self.__affineRegistrationComboBox = None
    self.__deformableRegistrationComboBox = None
    self.__interpolationComboBox = None
    self.__packageComboBox = None
    self.__updating = 0

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    infoLabel = qt.QLabel( 'If applicable, please select an atlas for each input dataset. Additionally, configure the registration speed and other parameters.' )

    # the registration parameters
    registrationParametersGroupBox = qt.QGroupBox()
    registrationParametersGroupBox.setTitle( 'Atlas-to-Dataset Registration Parameters' )
    self.__layout.addWidget( registrationParametersGroupBox )

    registrationParametersGroupBoxLayout = qt.QFormLayout( registrationParametersGroupBox )

    globalParametersNode = self.mrmlManager().GetGlobalParametersNode()

    # for all input channels add a qMRMLNodeComboBox
    for i in range( self.mrmlManager().GetTargetNumberOfSelectedVolumes() ):

      channelComboBox = slicer.qMRMLNodeComboBox()
      channelComboBox.setMRMLScene( slicer.mrmlScene )
      channelComboBox.nodeTypes = ['vtkMRMLScalarVolumeNode']
      channelComboBox.noneEnabled = True
      channelComboBox.addEnabled = False
      channelComboBox.removeEnabled = False
      channelComboBox.toolTip = 'Please select an atlas for this input dataset'
      registrationParametersGroupBoxLayout.addRow( globalParametersNode.GetNthTargetInputChannelName( i ), channelComboBox )
      channelComboBox.connect( 'currentNodeChanged(vtkMRMLNode*)', self.propagateToMRML )
      self.__channelComboBoxList.append( channelComboBox )


    # Affine Registration comboBox
    self.__affineRegistrationComboBox = qt.QComboBox()
    self.__affineRegistrationComboBox.addItems( Helper.GetRegistrationTypes() )
    self.__affineRegistrationComboBox.toolTip = 'Please configure the speed of the affine registration.'
    registrationParametersGroupBoxLayout.addRow( 'Affine Registration:', self.__affineRegistrationComboBox )
    self.__affineRegistrationComboBox.connect( 'currentIndexChanged(int)', self.propagateToMRML )

    # Deformable Registration comboBox
    self.__deformableRegistrationComboBox = qt.QComboBox()
    self.__deformableRegistrationComboBox.addItems( Helper.GetRegistrationTypes() )
    self.__deformableRegistrationComboBox.toolTip = 'Please configure the speed of the deformable registration.'
    registrationParametersGroupBoxLayout.addRow( 'Deformable Registration:', self.__deformableRegistrationComboBox )
    self.__deformableRegistrationComboBox.connect( 'currentIndexChanged(int)', self.propagateToMRML )

    # Interpolation
    self.__interpolationComboBox = qt.QComboBox()
    self.__interpolationComboBox.addItems( Helper.GetInterpolationTypes() )
    self.__interpolationComboBox.toolTip = 'Please configure the interpolation type. Linear is faster than Cubic.'
    registrationParametersGroupBoxLayout.addRow( 'Interpolation:', self.__interpolationComboBox )
    self.__interpolationComboBox.connect( 'currentIndexChanged(int)', self.propagateToMRML )

    # Package
    self.__packageComboBox = qt.QComboBox()
    self.__packageComboBox.addItems( Helper.GetPackages() )
    self.__packageComboBox.toolTip = 'Please select the registration package which should be used. If the package is not available on your system, BRAINS will be used.'
    registrationParametersGroupBoxLayout.addRow( 'Registration Package:', self.__packageComboBox )
    self.__packageComboBox.connect( 'currentIndexChanged(int)', self.propagateToMRML )


  def loadFromMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      if len( self.__channelComboBoxList ) > 0:
        # only do this if we have a channelComboBoxList defined
        for i in range( self.mrmlManager().GetTargetNumberOfSelectedVolumes() ):

          if not self.mrmlManager().GetGlobalParametersNode().GetRegistrationAtlasVolumeKey( i ):
            continue

          volumeID = self.mrmlManager().GetRegistrationAtlasVolumeID( i )

          volumeNodeID = self.mrmlManager().MapVTKNodeIDToMRMLNodeID( volumeID )
          if volumeNodeID:
            volumeNode = slicer.mrmlScene.GetNodeByID( volumeNodeID )
            if volumeNode:
              self.__channelComboBoxList[i].setCurrentNode( volumeNode )

      if self.__affineRegistrationComboBox:
        self.__affineRegistrationComboBox.setCurrentIndex( self.mrmlManager().GetRegistrationAffineType() )

      if self.__deformableRegistrationComboBox:
        self.__deformableRegistrationComboBox.setCurrentIndex( self.mrmlManager().GetRegistrationDeformableType() )

      if self.__interpolationComboBox:
        self.__interpolationComboBox.setCurrentIndex( self.mrmlManager().GetRegistrationInterpolationType() )

      if self.__packageComboBox:
        self.__packageComboBox.setCurrentIndex( self.mrmlManager().GetRegistrationPackageType() )

      self.__updating = 0

  def propagateToMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      for i in range( self.mrmlManager().GetTargetNumberOfSelectedVolumes() ):

        volumeNode = self.__channelComboBoxList[i].currentNode()
        if not volumeNode:
          volumeNodeID = None
        else:
          volumeNodeID = volumeNode.GetID()
          self.mrmlManager().SetRegistrationAtlasVolumeID( i, self.mrmlManager().MapMRMLNodeIDToVTKNodeID( volumeNodeID ) )

      self.mrmlManager().SetRegistrationAffineType( self.__affineRegistrationComboBox.currentIndex )
      self.mrmlManager().SetRegistrationDeformableType( self.__deformableRegistrationComboBox.currentIndex )
      self.mrmlManager().SetRegistrationInterpolationType( self.__interpolationComboBox.currentIndex )
      self.mrmlManager().SetRegistrationPackageType( self.__packageComboBox.currentIndex )

      self.__updating = 0

  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )
    # Load all values from MRML
    self.loadFromMRML()




  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )
