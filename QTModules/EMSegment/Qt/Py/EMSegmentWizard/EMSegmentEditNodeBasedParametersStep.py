from __main__ import qt, ctk
import PythonQt
import math

from EMSegmentStep import *
from Helper import *

class EMSegmentEditNodeBasedParametersStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '8. Edit Node-based Parameters' )
    self.setDescription( 'Specify node-based segmentation parameters.' )

    self.__parent = super( EMSegmentEditNodeBasedParametersStep, self )
    self.__layout = None
    self.__anatomicalTree = None
    self.__classLabel = None
    self.__classLabel2 = None
    self.__classLabel3 = None
    self.__classLabel4 = None
    self.__tabWidget = None

    self.__updating = 0
    self.__inputChannelWeights = []
    self.__classWeights = []
    self.__classWeightLayouts = []
    self.__classWeightLabels = []
    self.__classWeightSpinBoxes = []
    self.__classWeightCheckBoxes = []

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    self.__top = qt.QWidget()
    self.__topLayout = qt.QHBoxLayout( self.__top )

    # the anatomical tree
    self.__anatomicalTreeGroupBox = qt.QGroupBox()
    self.__anatomicalTreeGroupBox.setTitle( 'Anatomical Tree' )
    self.__topLayout.addWidget( self.__anatomicalTreeGroupBox )

    self.__anatomicalTreeGroupBoxLayout = qt.QFormLayout( self.__anatomicalTreeGroupBox )

    self.__anatomicalTree = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentAnatomicalTreeWidget()
    self.__anatomicalTree.structureNameEditable = False
    self.__anatomicalTree.labelColumnVisible = False
    self.__anatomicalTree.probabilityMapColumnVisible = False
    self.__anatomicalTree.classWeightColumnVisible = False
    self.__anatomicalTree.updateClassWeightColumnVisible = False
    self.__anatomicalTree.atlasWeightColumnVisible = False
    self.__anatomicalTree.alphaColumnVisible = False
    self.__anatomicalTree.displayAlphaCheckBoxVisible = False
    self.__anatomicalTree.connect( 'currentTreeNodeChanged(vtkMRMLNode*)', self.onTreeSelectionChanged )
    self.__anatomicalTree.setSizePolicy( qt.QSizePolicy.Expanding, qt.QSizePolicy.Expanding )
    self.__anatomicalTree.toolTip = 'Select an anatomical structure to configure the probablity in relation to other structures on the same level.'
    self.__anatomicalTree.setMinimumHeight( 200 )
    self.__anatomicalTreeGroupBoxLayout.addWidget( self.__anatomicalTree )

    #
    # overview of class weights panel
    #
    self.__overviewBox = qt.QGroupBox()
    self.__overviewBox.title = 'Overview of Class Weights'
    self.__overviewBox.toolTip = 'These are your guesses of probability relations between structures. Which structure takes how much percentage of the volume?'
    self.__overviewBoxLayout = qt.QVBoxLayout( self.__overviewBox )
    self.__topLayout.addWidget( self.__overviewBox )

    self.__layout.addWidget( self.__top )

    self.__tabWidget = qt.QTabWidget()
    self.__layout.addWidget( self.__tabWidget )

    #
    # basicPage
    #
    self.__basicPage = qt.QWidget()
    self.__basicPageLayout = qt.QHBoxLayout( self.__basicPage )

    self.__basicPageLeft = qt.QWidget()
    self.__basicPageRight = qt.QWidget()
    self.__basicPageLayoutLeft = qt.QFormLayout( self.__basicPageLeft )
    self.__basicPageLayoutRight = qt.QFormLayout( self.__basicPageRight )

    self.__basicPageLayout.addWidget( self.__basicPageLeft )
    self.__basicPageLayout.addWidget( self.__basicPageRight )

    self.__classLabel = qt.QLabel( "XX" )
    self.__basicPageLayoutLeft.addRow( "Class:", self.__classLabel )

    self.__classWeightSpinBox = qt.QDoubleSpinBox()
    self.__classWeightSpinBox.minimum = 0
    self.__classWeightSpinBox.maximum = 1
    self.__classWeightSpinBox.singleStep = 0.01
    self.__classWeightSpinBox.toolTip = 'Configure the class weight for the selected structure.'
    self.__basicPageLayoutLeft.addRow( "Class Weight:", self.__classWeightSpinBox )
    self.__classWeightSpinBox.connect( 'valueChanged(double)', self.propagateToMRML )

    self.__atlasWeightSpinBox = qt.QDoubleSpinBox()
    self.__atlasWeightSpinBox.minimum = 0
    self.__atlasWeightSpinBox.maximum = 1
    self.__atlasWeightSpinBox.singleStep = 0.01
    self.__atlasWeightSpinBox.toolTip = 'Configure the atlas weight for the selected structure.'
    self.__basicPageLayoutLeft.addRow( "Atlas Weight:", self.__atlasWeightSpinBox )
    self.__atlasWeightSpinBox.connect( 'valueChanged(double)', self.propagateToMRML )

    self.__mfaWeightSpinBox = qt.QDoubleSpinBox()
    self.__mfaWeightSpinBox.minimum = 0
    self.__mfaWeightSpinBox.maximum = 1
    self.__mfaWeightSpinBox.singleStep = 0.01
    self.__mfaWeightSpinBox.toolTip = 'Configure the MFA weight for the selected class.'
    self.__basicPageLayoutLeft.addRow( "MFA Weight:", self.__mfaWeightSpinBox )
    self.__mfaWeightSpinBox.connect( 'valueChanged(double)', self.propagateToMRML )

    self.__dummyLabel6 = qt.QLabel( "  " )
    self.__basicPageLayoutRight.addRow( "  ", self.__dummyLabel6 )

    self.__dummyLabel9 = qt.QLabel( "  " )
    self.__basicPageLayoutRight.addRow( "  ", self.__dummyLabel9 )

    self.__inputChannelWeightsBox = qt.QGroupBox()
    self.__inputChannelWeightsBox.title = 'Dataset Channel Weights'
    self.__inputChannelWeightsBox.toolTip = 'Configure different weights for each input dataset.'
    self.__inputChannelWeightsBoxLayout = qt.QFormLayout( self.__inputChannelWeightsBox )
    self.__basicPageLayoutRight.addWidget( self.__inputChannelWeightsBox )

    #
    # stoppingConditionsPage
    #
    self.__stoppingConditionsPage = qt.QWidget()
    self.__stoppingConditionsPageLayout = qt.QHBoxLayout( self.__stoppingConditionsPage )

    self.__stoppingConditionsPageLeft = qt.QWidget()
    self.__stoppingConditionsPageRight = qt.QWidget()
    self.__stoppingConditionsPageLayoutLeft = qt.QFormLayout( self.__stoppingConditionsPageLeft )
    self.__stoppingConditionsPageLayoutRight = qt.QFormLayout( self.__stoppingConditionsPageRight )

    self.__stoppingConditionsPageLayout.addWidget( self.__stoppingConditionsPageLeft )
    self.__stoppingConditionsPageLayout.addWidget( self.__stoppingConditionsPageRight )

    self.__classLabel2 = qt.QLabel( "XX" )
    self.__stoppingConditionsPageLayoutLeft.addRow( "Class:", self.__classLabel2 )

    self.__emComboBox = qt.QComboBox()
    self.__emComboBox.addItems( Helper.GetStoppingConditionTypes() )
    self.__emComboBox.toolTip = 'Configure the EM Stopping Condition for the selected class.'
    self.__stoppingConditionsPageLayoutLeft.addRow( 'EM:', self.__emComboBox )
    self.__emComboBox.connect( 'currentIndexChanged(int)', self.propagateToMRML )

    self.__mfaComboBox = qt.QComboBox()
    self.__mfaComboBox.addItems( Helper.GetStoppingConditionTypes() )
    self.__mfaComboBox.toolTip = 'Configure the MFA Stopping Condition for the selected class.'
    self.__stoppingConditionsPageLayoutLeft.addRow( 'MFA:', self.__mfaComboBox )
    self.__mfaComboBox.connect( 'currentIndexChanged(int)', self.propagateToMRML )

    self.__dummyLabel = qt.QLabel( "  " )
    self.__stoppingConditionsPageLayoutRight.addRow( "  ", self.__dummyLabel )

    self.__emValueSpinBox = qt.QSpinBox()
    self.__emValueSpinBox.minimum = 0
    self.__emValueSpinBox.singleStep = 1
    self.__emValueSpinBox.toolTip = 'Configure the value for the EM stopping condition.'
    self.__stoppingConditionsPageLayoutRight.addRow( 'Iterations:', self.__emValueSpinBox )
    self.__emValueSpinBox.connect( 'valueChanged(int)', self.propagateToMRML )

    self.__mfaValueSpinBox = qt.QSpinBox()
    self.__mfaValueSpinBox.minimum = 0
    self.__mfaValueSpinBox.singleStep = 1
    self.__mfaValueSpinBox.toolTip = 'Configure the MFA stopping condition. More iterations result in more smoothing.'
    self.__stoppingConditionsPageLayoutRight.addRow( 'Iterations:', self.__mfaValueSpinBox )
    self.__mfaValueSpinBox.connect( 'valueChanged(int)', self.propagateToMRML )

    self.__biasSpinBox = qt.QSpinBox()
    self.__biasSpinBox.minimum = -1
    self.__biasSpinBox.singleStep = 1
    self.__biasSpinBox.toolTip = 'Enable Bias Correction: -1, turn it off: 0.'
    self.__stoppingConditionsPageLayoutRight.addRow( 'Bias Iterations:', self.__biasSpinBox )
    self.__biasSpinBox.connect( 'valueChanged(int)', self.propagateToMRML )


    #
    # printPage
    #
    self.__printPage = qt.QWidget()
    self.__printPageLayout = qt.QHBoxLayout( self.__printPage )

    self.__printPageLeft = qt.QWidget()
    self.__printPageRight = qt.QWidget()
    self.__printPageLayoutLeft = qt.QFormLayout( self.__printPageLeft )
    self.__printPageLayoutRight = qt.QFormLayout( self.__printPageRight )

    self.__printPageLayout.addWidget( self.__printPageLeft )
    self.__printPageLayout.addWidget( self.__printPageRight )

    self.__classLabel3 = qt.QLabel( "XX" )
    self.__printPageLayoutLeft.addRow( "Class:", self.__classLabel3 )

    self.__weightCheckBox = qt.QCheckBox()
    self.__printPageLayoutLeft.addRow( "Weight:", self.__weightCheckBox )
    self.__weightCheckBox.toolTip = 'Toggle to print the weights.'
    self.__weightCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__qualityCheckBox = qt.QCheckBox()
    self.__printPageLayoutLeft.addRow( "Quality:", self.__qualityCheckBox )
    self.__qualityCheckBox.toolTip = 'Toggle to print the quality.'
    self.__qualityCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__frequencySpinBox = qt.QSpinBox()
    self.__frequencySpinBox.minimum = 0
    self.__frequencySpinBox.maximum = 20
    self.__frequencySpinBox.toolTip = 'Configure the print frequency.'
    self.__printPageLayoutLeft.addRow( "Frequency:", self.__frequencySpinBox )
    self.__frequencySpinBox.connect( 'valueChanged(int)', self.propagateToMRML )

    self.__biasCheckBox = qt.QCheckBox()
    self.__biasCheckBox.toolTip = 'Toggle to print the bias.'
    self.__printPageLayoutLeft.addRow( "Bias:", self.__biasCheckBox )
    self.__biasCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__labelMapCheckBox = qt.QCheckBox()
    self.__labelMapCheckBox.toolTip = 'Toggle to print the label map.'
    self.__printPageLayoutLeft.addRow( "Label Map:", self.__labelMapCheckBox )
    self.__labelMapCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__dummyLabel2 = qt.QLabel( "  " )
    self.__printPageLayoutRight.addRow( "  ", self.__dummyLabel2 )

    self.__dummyLabel3 = qt.QLabel( "  " )
    self.__printPageLayoutRight.addRow( "  ", self.__dummyLabel3 )

    self.__convergenceBox = qt.QGroupBox()
    self.__convergenceBox.title = 'Convergence'
    self.__convergenceBoxLayout = qt.QFormLayout( self.__convergenceBox )
    self.__printPageLayoutRight.addWidget( self.__convergenceBox )

    self.__convEMLabelMapCheckBox = qt.QCheckBox()
    self.__convEMLabelMapCheckBox.toolTip = 'Toggle to print the EM Label Map convergence.'
    self.__convergenceBoxLayout.addRow( "EM Label Map:", self.__convEMLabelMapCheckBox )
    self.__convEMLabelMapCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__convEMWeightsCheckBox = qt.QCheckBox()
    self.__convEMWeightsCheckBox.toolTip = 'Toggle to print the EM Weights convergence.'
    self.__convergenceBoxLayout.addRow( "EM Weights:", self.__convEMWeightsCheckBox )
    self.__convEMWeightsCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__convMFALabelMapCheckBox = qt.QCheckBox()
    self.__convEMWeightsCheckBox.toolTip = 'Toggle to print the MFA Label Map convergence.'
    self.__convergenceBoxLayout.addRow( "MFA Label Map:", self.__convMFALabelMapCheckBox )
    self.__convMFALabelMapCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__convMFAWeightsCheckBox = qt.QCheckBox()
    self.__convMFAWeightsCheckBox.toolTip = 'Toggle to print the MFA Weights convergence.'
    self.__convergenceBoxLayout.addRow( "MFA Weights:", self.__convMFAWeightsCheckBox )
    self.__convMFAWeightsCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )


    #
    # advancedPage
    #
    self.__advancedPage = qt.QWidget()
    self.__advancedPageLayout = qt.QFormLayout( self.__advancedPage )

    self.__dummyLabel14 = qt.QLabel( "  " )
    self.__advancedPageLayout.addWidget( self.__dummyLabel14 )

    self.__classLabel4 = qt.QLabel( "Class: XX" )
    self.__advancedPageLayout.addWidget( self.__classLabel4 )

#    self.__pcaParametersBox = qt.QGroupBox()
#    self.__pcaParametersBox.title = 'PCA Parameters'
#    self.__pcaParametersBoxLayout = qt.QFormLayout( self.__pcaParametersBox )
#    self.__advancedPageLayout.addWidget( self.__pcaParametersBox )
#
#    self.__registrationParametersBox = qt.QGroupBox()
#    self.__registrationParametersBox.title = 'Registration Parameters'
#    self.__registrationParametersBoxLayout = qt.QFormLayout( self.__registrationParametersBox )
#    self.__advancedPageLayout.addWidget( self.__registrationParametersBox )

    self.__miscParametersBox = qt.QGroupBox()
    self.__miscParametersBox.title = 'Miscellaneous Parameters'
    self.__miscParametersBoxLayout = qt.QFormLayout( self.__miscParametersBox )
    self.__advancedPageLayout.addWidget( self.__miscParametersBox )

    self.__excludeFromEStepCheckBox = qt.QCheckBox()
    self.__excludeFromEStepCheckBox.toolTip = 'Toggle to exclude from Incomplete EStep.'
    self.__miscParametersBoxLayout.addRow( "Exclude From Incomplete EStep:", self.__excludeFromEStepCheckBox )
    self.__excludeFromEStepCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__genBackgroundProbCheckBox = qt.QCheckBox()
    self.__genBackgroundProbCheckBox.toolTip = 'Toggle to detect the background value.'
    self.__miscParametersBoxLayout.addRow( "Generate Background Probability:", self.__genBackgroundProbCheckBox )
    self.__genBackgroundProbCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

    self.__meanFieldParametersBox = qt.QGroupBox()
    self.__meanFieldParametersBox.title = 'Mean Field Parameters'
    self.__meanFieldParametersBoxLayout = qt.QFormLayout( self.__meanFieldParametersBox )
    self.__advancedPageLayout.addWidget( self.__meanFieldParametersBox )

    self.__twoDNeighborhoodCheckBox = qt.QCheckBox()
    self.__twoDNeighborhoodCheckBox.toolTip = 'Toggle to use 2D Neighborhood.'
    self.__meanFieldParametersBoxLayout.addRow( "2D Neighborhood:", self.__twoDNeighborhoodCheckBox )
    self.__twoDNeighborhoodCheckBox.connect( 'stateChanged(int)', self.propagateToMRML )

#    self.__inhomogeneityParametersBox = qt.QGroupBox()
#    self.__inhomogeneityParametersBox.title = 'Inhomogeneity Parameters'
#    self.__inhomogeneityParametersBoxLayout = qt.QFormLayout( self.__inhomogeneityParametersBox )
#    self.__advancedPageLayout.addWidget( self.__inhomogeneityParametersBox )


    #
    ### add all tabs to the tabWidget
    #
    self.__tabWidget.addTab( self.__basicPage, "Basic" )
    self.__tabWidget.addTab( self.__stoppingConditionsPage, "Stopping Conditions" )
    self.__tabWidget.addTab( self.__printPage, "Print" )
    self.__tabWidget.addTab( self.__advancedPage, "Advanced" )




  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    if self.__anatomicalTree:
      self.__anatomicalTree.setMRMLManager( self.mrmlManager() )
      self.__anatomicalTree.updateWidgetFromMRML()

    # reset the panel
    self.loadFromMRML()


  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    if self.mrmlManager().GetTreeRootNode:

      firstBadTreeID = self.mrmlManager().GetTreeNodeFirstIDWithChildProbabilityError()

      if firstBadTreeID >= 0:

        message = "Child probabilities must sum to one for node "
        message += str( self.mrmlManager().GetTreeNodeName( firstBadTreeID ) )
        message += "; right now they sum to "
        message += str( self.mrmlManager().GetTreeNodeChildrenSumClassProbability( firstBadTreeID ) )
        message += ".  Please fix before continuing---"
        message += "you should edit the \"Class Weight\" fields for the"
        message += " children nodes of "
        message += str( self.mrmlManager().GetTreeNodeName( firstBadTreeID ) ) + "."

        self.__parent.validationFailed( desiredBranchId, 'Node Parameters Error', message, True )
      else:
        self.__parent.validationSucceeded( desiredBranchId )


  def onTreeSelectionChanged( self ):
    '''
    '''

    mrmlNode = self.__anatomicalTree.currentNode()

    if mrmlNode:
      self.loadFromMRML( mrmlNode )


  def propagateToMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      # save the values to MRML
      mrmlNode = self.__anatomicalTree.currentNode()

      if mrmlNode:
        vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )

        globalNode = self.mrmlManager().GetGlobalParametersNode()
        if not globalNode:
          Helper.Error( 'No global node!' )
          return

        # check if we have a leaf
        isLeaf = self.mrmlManager().GetTreeNodeIsLeaf( vtkId )

        #
        # basic panel
        #
        self.mrmlManager().SetTreeNodeClassProbability( vtkId, self.__classWeightSpinBox.value )
        self.mrmlManager().SetTreeNodeSpatialPriorWeight( vtkId, self.__atlasWeightSpinBox.value )

        if not isLeaf:
          self.__mfaWeightSpinBox.setEnabled( True )
          self.mrmlManager().SetTreeNodeAlpha( vtkId, self.__mfaWeightSpinBox.value )
        else:
          self.__mfaWeightSpinBox.value = 0
          self.__mfaWeightSpinBox.setEnabled( False )

        ## input channel weights
        curVolumeIndex = 0
        for i in self.__inputChannelWeights:
          curVolumeIndex = curVolumeIndex + 1
          self.mrmlManager().SetTreeNodeInputChannelWeight( vtkId, curVolumeIndex, i.value )


        # 
        # stopping conditions panel
        #
        if isLeaf:
          self.__tabWidget.setTabEnabled( 1, False )
        else:
          # only show if we did not select a leaf
          self.__tabWidget.setTabEnabled( 1, True )

          selectionChanged = self.mrmlManager().GetTreeNodeStoppingConditionEMType( vtkId ) != self.__emComboBox.currentIndex
          self.mrmlManager().SetTreeNodeStoppingConditionEMType( vtkId, self.__emComboBox.currentIndex )
          if self.__emComboBox.currentIndex == 0:
            # iteration selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__emValueSpinBox ).setText( 'Iterations:' )
            if selectionChanged:
              # if the combobox changed, we do not set the new value but read it instead
              self.__emValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionEMIterations( vtkId )
            else:
              self.mrmlManager().SetTreeNodeStoppingConditionEMIterations( vtkId, self.__emValueSpinBox.value )
          elif self.__emComboBox.currentIndex >= 1:
            # labelMap or value selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__emValueSpinBox ).setText( 'Value:' )
            if selectionChanged:
              # if the combobox changed, we do not set the new value but read it instead
              self.__emValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionEMValue( vtkId )
            else:
              self.mrmlManager().SetTreeNodeStoppingConditionEMValue( vtkId, self.__emValueSpinBox.value )

          selectionChanged = self.mrmlManager().GetTreeNodeStoppingConditionMFAType( vtkId ) != self.__mfaComboBox.currentIndex
          self.mrmlManager().SetTreeNodeStoppingConditionMFAType( vtkId, self.__mfaComboBox.currentIndex )
          if self.__mfaComboBox.currentIndex == 0:
            # iteration selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__mfaValueSpinBox ).setText( 'Iterations:' )
            if selectionChanged:
              # if the combobox changed, we do not set the new value but read it instead
              self.__mfaValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionMFAIterations( vtkId )
            else:
              self.mrmlManager().SetTreeNodeStoppingConditionMFAIterations( vtkId, self.__mfaValueSpinBox.value )
          elif self.__mfaComboBox.currentIndex >= 1:
            # labelMap or value selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__mfaValueSpinBox ).setText( 'Value:' )
            if selectionChanged:
              # if the combobox changed, we do not set the new value but read it instead
              self.__mfaValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionMFAValue( vtkId )
            else:
              self.mrmlManager().SetTreeNodeStoppingConditionMFAValue( vtkId, self.__mfaValueSpinBox.value )



        #
        # print panel
        #
        if isLeaf:
          self.mrmlManager().SetTreeNodePrintWeight( vtkId, int( self.__weightCheckBox.checked ) )
          self.mrmlManager().SetTreeNodePrintQuality( vtkId, int( self.__qualityCheckBox.checked ) )

        else:
          self.mrmlManager().SetTreeNodePrintWeight( vtkId, int( self.__weightCheckBox.checked ) )
          self.mrmlManager().SetTreeNodePrintFrequency( vtkId, int( self.__frequencySpinBox.value ) )
          self.mrmlManager().SetTreeNodePrintBias( vtkId, int( self.__biasCheckBox.checked ) )
          self.mrmlManager().SetTreeNodePrintLabelMap( vtkId, int( self.__labelMapCheckBox.checked ) )

          self.mrmlManager().SetTreeNodePrintEMLabelMapConvergence( vtkId, int( self.__convEMLabelMapCheckBox.checked ) )
          self.mrmlManager().SetTreeNodePrintEMWeightsConvergence( vtkId, int( self.__convEMWeightsCheckBox.checked ) )
          self.mrmlManager().SetTreeNodePrintMFALabelMapConvergence( vtkId, int( self.__convMFALabelMapCheckBox.checked ) )
          self.mrmlManager().SetTreeNodePrintMFAWeightsConvergence( vtkId, int( self.__convMFAWeightsCheckBox.checked ) )


        #
        # Advanced panel
        #
        if not isLeaf:
          self.mrmlManager().SetTreeNodeExcludeFromIncompleteEStep( vtkId, int( self.__excludeFromEStepCheckBox.checked ) )
          self.mrmlManager().SetTreeNodeGenerateBackgroundProbability( vtkId, int( self.__genBackgroundProbCheckBox.checked ) )
          self.mrmlManager().SetTreeNodeInteractionMatrices2DFlag( vtkId, int( self.__twoDNeighborhoodCheckBox.checked ) )


      self.__updating = 0


  def loadFromMRML( self, mrmlNode=None ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      self.resetPanel()

      if mrmlNode:

        # adjust the panel to the given mrmlNode
        vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )

        globalNode = self.mrmlManager().GetGlobalParametersNode()
        if not globalNode:
          Helper.Error( 'No global node!' )
          return

        # check if we have a leaf
        isLeaf = self.mrmlManager().GetTreeNodeIsLeaf( vtkId )

        # re-enable the widgets
        self.resetPanel( True )

        self.__classLabel.setText( mrmlNode.GetName() )
        self.__classLabel2.setText( mrmlNode.GetName() )
        self.__classLabel3.setText( mrmlNode.GetName() )
        self.__classLabel4.setText( "Class: " + mrmlNode.GetName() )

        #
        # basic panel
        #
        self.__classWeightSpinBox.value = self.mrmlManager().GetTreeNodeClassProbability( vtkId )
        self.__atlasWeightSpinBox.value = self.mrmlManager().GetTreeNodeSpatialPriorWeight( vtkId )
        if not isLeaf:
          self.__mfaWeightSpinBox.setEnabled( True )
          self.__mfaWeightSpinBox.value = self.mrmlManager().GetTreeNodeAlpha( vtkId )
        else:
          self.__mfaWeightSpinBox.value = 0
          self.__mfaWeightSpinBox.setEnabled( False )

        ## input channel weights
        # 1. clear all
        for i in self.__inputChannelWeights:
          l = self.__inputChannelWeightsBoxLayout.labelForField( i )
          self.__inputChannelWeightsBoxLayout.removeWidget( l )
          l.deleteLater()
          l.setParent( None )
          l = None
          self.__inputChannelWeightsBoxLayout.removeWidget( i )
          i.deleteLater()
          i.setParent( None )
          i = None
        self.__inputChannelWeights = []
        # 2. re-create all
        numberOfVolumes = self.mrmlManager().GetTargetNumberOfSelectedVolumes()
        for i in range( numberOfVolumes ):
          # create new spinbox
          currentSpinBox = qt.QDoubleSpinBox()
          currentSpinBox.minimum = 0
          currentSpinBox.maximum = 1
          currentSpinBox.singleStep = 0.01
          currentSpinBox.value = self.mrmlManager().GetTreeNodeInputChannelWeight( vtkId, i )
          self.__inputChannelWeights.append( currentSpinBox )
          self.__inputChannelWeights[-1].connect( 'valueChanged(double)', self.propagateToMRML )
          self.__inputChannelWeightsBoxLayout.addRow( globalNode.GetNthTargetInputChannelName( i ), self.__inputChannelWeights[-1] )

        # 
        # stopping conditions panel
        #
        if isLeaf:
          self.__tabWidget.setTabEnabled( 1, False )
        else:
          # only show if we did not select a leaf
          self.__tabWidget.setTabEnabled( 1, True )

          self.__emComboBox.setCurrentIndex( self.mrmlManager().GetTreeNodeStoppingConditionEMType( vtkId ) )
          if self.__emComboBox.currentIndex == 0:
            # iteration selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__emValueSpinBox ).setText( 'Iterations:' )
            self.__emValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionEMIterations( vtkId )
          elif self.__emComboBox.currentIndex >= 1:
            # labelMap or value selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__emValueSpinBox ).setText( 'Value:' )
            self.__emValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionEMValue( vtkId )

          self.__mfaComboBox.setCurrentIndex( self.mrmlManager().GetTreeNodeStoppingConditionMFAType( vtkId ) )
          if self.__mfaComboBox.currentIndex == 0:
            # iteration selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__mfaValueSpinBox ).setText( 'Iterations:' )
            self.__mfaValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionMFAIterations( vtkId )
          elif self.__mfaComboBox.currentIndex >= 1:
            # labelMap or value selected
            self.__stoppingConditionsPageLayoutRight.labelForField( self.__mfaValueSpinBox ).setText( 'Value:' )
            self.__mfaValueSpinBox.value = self.mrmlManager().GetTreeNodeStoppingConditionMFAValue( vtkId )

          self.__biasSpinBox.value = self.mrmlManager().GetTreeNodeBiasCalculationMaxIterations( vtkId )

        #
        # print panel
        #
        if isLeaf:

          self.__weightCheckBox.show()
          self.__printPageLayoutLeft.labelForField( self.__qualityCheckBox ).show()
          self.__qualityCheckBox.show()
          self.__printPageLayoutLeft.labelForField( self.__frequencySpinBox ).hide()
          self.__frequencySpinBox.hide()
          self.__printPageLayoutLeft.labelForField( self.__biasCheckBox ).hide()
          self.__biasCheckBox.hide()
          self.__printPageLayoutLeft.labelForField( self.__labelMapCheckBox ).hide()
          self.__labelMapCheckBox.hide()
          self.__convergenceBox.hide()

          self.__weightCheckBox.checked = self.mrmlManager().GetTreeNodePrintWeight( vtkId )
          self.__qualityCheckBox.checked = self.mrmlManager().GetTreeNodePrintQuality( vtkId )

        else:
          self.__weightCheckBox.show()
          self.__printPageLayoutLeft.labelForField( self.__qualityCheckBox ).hide()
          self.__qualityCheckBox.hide()
          self.__printPageLayoutLeft.labelForField( self.__frequencySpinBox ).show()
          self.__frequencySpinBox.show()
          self.__printPageLayoutLeft.labelForField( self.__biasCheckBox ).show()
          self.__biasCheckBox.show()
          self.__printPageLayoutLeft.labelForField( self.__labelMapCheckBox ).show()
          self.__labelMapCheckBox.show()
          self.__convergenceBox.show()

          self.__weightCheckBox.checked = self.mrmlManager().GetTreeNodePrintWeight( vtkId )
          self.__frequencySpinBox.value = self.mrmlManager().GetTreeNodePrintFrequency( vtkId )
          self.__biasCheckBox.checked = self.mrmlManager().GetTreeNodePrintBias( vtkId )
          self.__labelMapCheckBox.checked = self.mrmlManager().GetTreeNodePrintLabelMap( vtkId )

          self.__convEMLabelMapCheckBox.checked = self.mrmlManager().GetTreeNodePrintEMLabelMapConvergence( vtkId )
          self.__convEMWeightsCheckBox.checked = self.mrmlManager().GetTreeNodePrintEMWeightsConvergence( vtkId )
          self.__convMFALabelMapCheckBox.checked = self.mrmlManager().GetTreeNodePrintMFALabelMapConvergence( vtkId )
          self.__convMFAWeightsCheckBox.checked = self.mrmlManager().GetTreeNodePrintMFAWeightsConvergence( vtkId )

        #
        # Advanced panel
        #
        if isLeaf:
          self.__tabWidget.setTabEnabled( 3, False )
          self.__miscParametersBoxLayout.labelForField( self.__excludeFromEStepCheckBox ).hide()
          self.__excludeFromEStepCheckBox.hide()
          self.__miscParametersBoxLayout.labelForField( self.__genBackgroundProbCheckBox ).hide()
          self.__genBackgroundProbCheckBox.hide()
          self.__meanFieldParametersBoxLayout.labelForField( self.__twoDNeighborhoodCheckBox ).hide()
          self.__twoDNeighborhoodCheckBox.hide()
        else:
          self.__tabWidget.setTabEnabled( 3, True )
          self.__miscParametersBoxLayout.labelForField( self.__excludeFromEStepCheckBox ).show()
          self.__excludeFromEStepCheckBox.show()
          self.__miscParametersBoxLayout.labelForField( self.__genBackgroundProbCheckBox ).show()
          self.__genBackgroundProbCheckBox.show()
          self.__meanFieldParametersBoxLayout.labelForField( self.__twoDNeighborhoodCheckBox ).show()
          self.__twoDNeighborhoodCheckBox.show()

          self.__excludeFromEStepCheckBox.checked = self.mrmlManager().GetTreeNodeExcludeFromIncompleteEStep( vtkId )
          self.__genBackgroundProbCheckBox.checked = self.mrmlManager().GetTreeNodeGenerateBackgroundProbability( vtkId )
          self.__twoDNeighborhoodCheckBox.checked = self.mrmlManager().GetTreeNodeInteractionMatrices2DFlag( vtkId )


        #
        # overview panel
        #

        # first, clear it all
        for i in self.__classWeights:
          self.__overviewBoxLayout.removeWidget( i )
          i.deleteLater()
          i.setParent( None )
          i = None

        self.__classWeights = []
        self.__classWeightLayouts = []
        self.__classWeightLabels = []
        self.__classWeightSpinBoxes = []
        self.__classWeightCheckBoxes = []

        # fill it again
        if vtkId != self.mrmlManager().GetTreeRootNodeID():
          # other node than root selected
          parent = self.mrmlManager().GetTreeNodeParentNodeID( vtkId )
          numberOfChildren = self.mrmlManager().GetTreeNodeNumberOfChildren( parent )
          for i in range( numberOfChildren ):
            child = self.mrmlManager().GetTreeNodeChildNodeID( parent, i )
            name = self.mrmlManager().GetTreeNodeName( child )
            probability = self.mrmlManager().GetTreeNodeClassProbability( child )

            label = qt.QLabel( name + ":" )

            spinBox = qt.QDoubleSpinBox()
            spinBox.minimum = 0
            spinBox.maximum = 1
            spinBox.singleStep = 0.01
            spinBox.value = probability


            checkBox = qt.QCheckBox( "  " )
            checkBox.toolTip = 'Toggle for auto-update when changing other weights.'

            self.__classWeightLabels.append( label )
            self.__classWeightSpinBoxes.append( spinBox )
            self.__classWeightCheckBoxes.append( checkBox )

            self.__classWeightSpinBoxes[-1].connect( 'valueChanged(double)', self.updateClassWeights )
            self.__classWeightCheckBoxes[-1].connect( 'stateChanged(int)', self.updateClassWeights )

            weightRow = qt.QWidget()
            weightRowLayout = qt.QHBoxLayout( weightRow )
            weightRowLayout.addWidget( self.__classWeightLabels[-1] )
            weightRowLayout.addWidget( self.__classWeightSpinBoxes[-1] )
            weightRowLayout.addWidget( self.__classWeightCheckBoxes[-1] )

            self.__classWeights.append( weightRow )
            self.__classWeightLayouts.append( weightRowLayout )

            self.__overviewBoxLayout.addWidget( self.__classWeights[-1], 0, 2 )



      self.__updating = 0

  def updateClassWeights( self ):
    '''
    '''

    if not self.__updating:

      # save the values to MRML
      mrmlNode = self.__anatomicalTree.currentNode()

      if mrmlNode:
        self.__updating = 1

        vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )

        if self.mrmlManager().GetTreeRootNodeID() == vtkId:
          self.__updating = 0
          return

        parent = self.mrmlManager().GetTreeNodeParentNodeID( vtkId )

        if not parent:
          self.__updating = 0
          return

        numberOfChildren = self.mrmlManager().GetTreeNodeNumberOfChildren( parent )

        # update all probabilites
        for i in range( numberOfChildren ):
          child = self.mrmlManager().GetTreeNodeChildNodeID( parent, i )

          value = self.__classWeightSpinBoxes[i].value

          self.mrmlManager().SetTreeNodeClassProbability( child, value )

        normProb = self.mrmlManager().GetTreeNodeChildrenSumClassProbability( parent )


        if normProb == 0 or ( math.fabs( 1 - normProb ) < 0.00005 ):
          # we don't have to do anything!
          self.__updating = 0
          return


        # start normalizing
        fixedProb = 0.0
        flexProb = 0.0

        autoState = [0] * numberOfChildren
        autoStateFlag = 0
        prob = [0] * numberOfChildren
        childID = [0] * numberOfChildren

        for i in range( numberOfChildren ):

          if self.__classWeightSpinBoxes[i] and self.__classWeightCheckBoxes[i].checked:

            autoState[i] = 1
            autoStateFlag = 1

          else:

            autoState[i] = 0

          childID[i] = self.mrmlManager().GetTreeNodeChildNodeID( parent, i )
          prob[i] = self.mrmlManager().GetTreeNodeClassProbability( childID[i] )

          if autoState[i]:
            flexProb += prob[i]
          else:
            fixedProb += prob[i]

        if autoStateFlag:

          leftProb = 1 - fixedProb
          if leftProb < 0:

            for i in range( numberOfChildren ):

              if autoState[i]:
                self.mrmlManager().SetTreeNodeClassProbability( childID[i], 0.0 )


          elif flexProb > 0:

            norm = leftProb / flexProb
            for i in range( numberOfChildren ):

              if autoState[i]:
                self.mrmlManager().SetTreeNodeClassProbability( childID[i], norm * prob[i] )

          elif leftProb > 0:

            index = 0
            while not autoState[index] and index < numberOfChildren:
              index = index + 1

            if index < numberOfChildren:
              self.mrmlManager().SetTreeNodeClassProbability( childID[index], leftProb )


        # update all probabilites
        for i in range( numberOfChildren ):
          child = self.mrmlManager().GetTreeNodeChildNodeID( parent, i )

          value = self.mrmlManager().GetTreeNodeClassProbability( child )
          self.__classWeightSpinBoxes[i].value = value

        self.__classWeightSpinBox.value = self.mrmlManager().GetTreeNodeClassProbability( vtkId )

        self.__updating = 0



  def resetPanel( self, enabled=False ):
    '''
    '''
    if self.__classLabel:
      self.__classLabel.setText( '' )

    if self.__classLabel2:
      self.__classLabel2.setText( '' )

    if self.__classLabel3:
      self.__classLabel3.setText( '' )

    if self.__classLabel4:
      self.__classLabel4.setText( '' )



    if self.__tabWidget:
      self.__tabWidget.setEnabled( enabled )
      self.__tabWidget.setTabEnabled( 1, False )
      self.__tabWidget.setTabEnabled( 3, False )
