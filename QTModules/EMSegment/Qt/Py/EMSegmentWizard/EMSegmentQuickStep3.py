from __main__ import qt, ctk
import PythonQt

import math

from EMSegmentStep import *
from Helper import *

class EMSegmentQuickStep3( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '3. Sampling and Class Weights' )
    self.setDescription( 'Define seeds for each anatomical structure and define probability relations between them.' )

    self.__parent = super( EMSegmentQuickStep3, self )
    self.__layout = None
    self.__anatomicalTree = None
    self.__classLabel = None
    self.__classLabel2 = None
    self.__specificationComboBox = None
    self.__meanMatrixWidget = None
    self.__logCovarianceMatrixWidget = None
    self.__resetDistributionButton = None
    self.__tabWidget = None

    self.__updating = 0
    self.__manualSampling = False
    self.__vtkId = None

    self.__inputChannelWeights = []
    self.__classWeights = []
    self.__classWeightLayouts = []
    self.__classWeightLabels = []
    self.__classWeightSpinBoxes = []
    self.__classWeightCheckBoxes = []

    slicer.sliceWidgetRed_interactorStyle.AddObserver( vtk.vtkCommand.LeftButtonReleaseEvent, self.onClickInRedSliceView )
    slicer.sliceWidgetYellow_interactorStyle.AddObserver( vtk.vtkCommand.LeftButtonReleaseEvent, self.onClickInYellowSliceView )
    slicer.sliceWidgetGreen_interactorStyle.AddObserver( vtk.vtkCommand.LeftButtonReleaseEvent, self.onClickInGreenSliceView )

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    self.__top = qt.QWidget()
    self.__topLayout = qt.QHBoxLayout( self.__top )

    # the anatomical tree
    anatomicalTreeGroupBox = qt.QGroupBox()
    anatomicalTreeGroupBox.setTitle( 'Anatomical Tree' )
    self.__topLayout.addWidget( anatomicalTreeGroupBox )

    anatomicalTreeGroupBoxLayout = qt.QFormLayout( anatomicalTreeGroupBox )

    self.__anatomicalTree = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentAnatomicalTreeWidget()
    self.__anatomicalTree.structureNameEditable = False
    self.__anatomicalTree.labelColumnVisible = False
    self.__anatomicalTree.probabilityMapColumnVisible = False
    self.__anatomicalTree.classWeightColumnVisible = False
    self.__anatomicalTree.updateClassWeightColumnVisible = False
    self.__anatomicalTree.atlasWeightColumnVisible = False
    self.__anatomicalTree.alphaColumnVisible = False
    self.__anatomicalTree.displayAlphaCheckBoxVisible = False
    self.__anatomicalTree.setMinimumHeight( 200 )
    self.__anatomicalTree.toolTip = 'Select a structure to configure the intensity distribution.'
    self.__anatomicalTree.setSizePolicy( qt.QSizePolicy.MinimumExpanding, qt.QSizePolicy.MinimumExpanding )
    self.__anatomicalTree.connect( 'currentTreeNodeChanged(vtkMRMLNode*)', self.onTreeSelectionChanged )
    anatomicalTreeGroupBoxLayout.addWidget( self.__anatomicalTree )

    #
    # overview of class weights panel
    #
    self.__overviewBox = qt.QGroupBox()
    self.__overviewBox.title = 'Guesses of Probability'
    self.__overviewBox.toolTip = 'These are your guesses of probability relations between structures. Which structure takes how much percentage of the volume?'
    self.__overviewBoxLayout = qt.QVBoxLayout( self.__overviewBox )
    self.__topLayout.addWidget( self.__overviewBox )

    self.__layout.addWidget( self.__top )

    self.__tabWidget = qt.QTabWidget()
    self.__layout.addWidget( self.__tabWidget )

    #
    # manualSamplingPage
    #
    manualSamplingPage = qt.QWidget()
    manualSamplingPageLayout = qt.QFormLayout( manualSamplingPage )

    self.__classLabel2 = qt.QLabel( "Class: XX" )
    manualSamplingPageLayout.addWidget( self.__classLabel2 )

    self.__infoLabel = qt.QLabel( "left mouse Click in a slice window to pick a sample" )
    manualSamplingPageLayout.addWidget( self.__infoLabel )

    self.__manualSampleTable = qt.QTableWidget()
    manualSamplingPageLayout.addWidget( self.__manualSampleTable )

    self.__tabWidget.addTab( manualSamplingPage, "Manual Sampling" )

    self.__plotDistributionButton = qt.QPushButton()
    self.__plotDistributionButton.text = "Plot Distribution"
    self.__plotDistributionButton.toolTip = 'Click to plot the intensity distributions for all structures.'
    self.__layout.addRow( self.__plotDistributionButton )
    self.__plotDistributionButton.connect( 'clicked()', self.plotDistribution )

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

    self.__parent.validationSucceeded( desiredBranchId )

  def plotDistribution( self ):
    '''
    '''
    self.__d = qt.QDialog()
    self.__dLayout = qt.QHBoxLayout( self.__d )
    self.__graphWidget = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentGraphWidget()
    self.__graphWidget.setMRMLManager( self.mrmlManager() )
    self.__dLayout.addWidget( self.__graphWidget )
    self.__d.setModal( True )
    self.__d.show()


  def resetDistribution( self ):
    '''
    '''
    mrmlNode = self.__anatomicalTree.currentNode()

    if mrmlNode:
      vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )
      # check if we have a leaf
      isLeaf = self.mrmlManager().GetTreeNodeIsLeaf( vtkId )

      if isLeaf:
        self.mrmlManager().ResetTreeNodeDistributionLogMeanCorrection( vtkId );
        self.mrmlManager().ResetTreeNodeDistributionLogCovarianceCorrection( vtkId );
        self.loadFromMRML( mrmlNode )

  def onTreeSelectionChanged( self ):
    '''
    '''

    mrmlNode = self.__anatomicalTree.currentNode()

    if mrmlNode:
      vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )
      # check if we have a leaf
      isLeaf = self.mrmlManager().GetTreeNodeIsLeaf( vtkId )

      if isLeaf:
        self.loadFromMRML( mrmlNode )
      else:
        self.loadFromMRML( None )


  def propagateToMRML( self ):
    '''
    '''
    if not self.__updating:

      self.__updating = 1

      # save the values to MRML
      mrmlNode = self.__anatomicalTree.currentNode()

      if mrmlNode:
        vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )

        self.mrmlManager().SetTreeNodeDistributionSpecificationMethod( vtkId, self.__specificationComboBox.currentIndex )

        # manual sampling, en-/disable the manual sampling tab
        self.__tabWidget.setTabEnabled( 1, ( self.__specificationComboBox.currentIndex == 1 ) )
        if self.__specificationComboBox.currentIndex == 1:
          self.setupManualSampleTable( mrmlNode )


        # number of volumes
        numberOfVolumes = self.mrmlManager().GetTargetNumberOfSelectedVolumes()

        # propagate the matrix values
        for c in range( numberOfVolumes ):
          value = self.__meanMatrixWidget.value( 0, c )
          self.mrmlManager().SetTreeNodeDistributionMeanWithCorrection( vtkId, c, value )

        for r in range( numberOfVolumes ):
          for c in range( numberOfVolumes ):
            value = self.__logCovarianceMatrixWidget.value( r, c )
            self.mrmlManager().SetTreeNodeDistributionLogCovariance( vtkId, r, c, value )



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

        # number of volumes
        numberOfVolumes = self.mrmlManager().GetTargetNumberOfSelectedVolumes()

        # re-enable the widgets
        self.resetPanel( True )

        self.__classLabel2.setText( "Class: " + mrmlNode.GetName() )

        self.setupManualSampleTable( mrmlNode )

        for c in range( numberOfVolumes ):

          value = self.mrmlManager().GetTreeNodeDistributionMeanWithCorrection( vtkId, c )

        for r in range( numberOfVolumes ):
          for c in range( numberOfVolumes ):
            value = self.mrmlManager().GetTreeNodeDistributionLogCovarianceWithCorrection( vtkId, r, c )

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

  def setupManualSampleTable( self, mrmlNode ):
    '''
    '''
    vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )

    self.mrmlManager().SetTreeNodeDistributionSpecificationMethod( vtkId, 1 )

    # number of volumes
    numberOfVolumes = self.mrmlManager().GetTargetNumberOfSelectedVolumes()
    numberOfSamples = self.mrmlManager().GetTreeNodeDistributionNumberOfSamples( vtkId )

    # empty all old items, we need a class variable here to store the references
    # if not, the items will be all lost!!
    self.__items = []

    # erase all items in the manual sampling table
    for r in range( self.__manualSampleTable.rowCount ):
      self.__manualSampleTable.removeRow( r )
    for c in range( self.__manualSampleTable.columnCount ):
      self.__manualSampleTable.removeColumn( c )

    self.__manualSampleTable.rowCount = 0
    self.__manualSampleTable.columnCount = 0

    # configure the rows and columns of the table
    volumes = []
    for n in range( numberOfVolumes ):
      # save the columns
      volumeId = self.mrmlManager().GetTargetSelectedVolumeNthID( n )
      volumeName = self.mrmlManager().GetVolumeName( volumeId )
      volumes.append( volumeName )

      # create new column
      self.__manualSampleTable.insertColumn( self.__manualSampleTable.columnCount )

    self.__manualSampleTable.setHorizontalHeaderLabels( volumes )

    # add the manual sample intensity values to the table
    for m in range( numberOfSamples ):

      # create new row
      self.__manualSampleTable.insertRow( self.__manualSampleTable.rowCount )

      for n in range( numberOfVolumes ):

        # fill the new row

        volumeId = self.mrmlManager().GetTargetSelectedVolumeNthID( n )
        intensity = self.mrmlManager().GetTreeNodeDistributionSampleIntensityValue( vtkId, m, volumeId )

        self.__items.append( qt.QTableWidgetItem( str( intensity ) ) )
        self.__manualSampleTable.setItem( m, n, self.__items[-1] )

    self.mrmlManager().ResetTreeNodeDistributionLogMeanCorrection( vtkId );
    self.mrmlManager().ResetTreeNodeDistributionLogCovarianceCorrection( vtkId );

    self.__vtkId = vtkId

    self.__manualSampling = True


  def resetPanel( self, enabled=False ):
    '''
    '''

    if self.__classLabel2:
      self.__classLabel2.setText( '' )

    if self.__tabWidget:
      self.__tabWidget.setEnabled( enabled )

    self.__manualSampling = False


  def onClickInRedSliceView( self, interactorStyleTrackballCamera, event ):
    '''
    '''
    if not self.__manualSampling:
      return

    rasPos = Helper.onClickInSliceView( slicer.sliceWidgetRed_interactorStyle, slicer.sliceWidgetRed_sliceLogic )
    mrmlNode = self.__anatomicalTree.currentNode()
    self.mrmlManager().AddTreeNodeDistributionSamplePoint( self.__vtkId, rasPos );

    self.loadFromMRML( mrmlNode )
    self.__tabWidget.setCurrentIndex( 1 )

    Helper.Debug( ' RED RAS: ' + str( rasPos ) )

  def onClickInYellowSliceView( self, interactorStyleTrackballCamera, event ):
    '''
    '''
    if not self.__manualSampling:
      return

    rasPos = Helper.onClickInSliceView( slicer.sliceWidgetYellow_interactorStyle, slicer.sliceWidgetYellow_sliceLogic )
    mrmlNode = self.__anatomicalTree.currentNode()
    self.mrmlManager().AddTreeNodeDistributionSamplePoint( self.__vtkId, rasPos );

    self.loadFromMRML( mrmlNode )
    self.__tabWidget.setCurrentIndex( 1 )

    Helper.Debug( ' YELLOW RAS: ' + str( rasPos ) )

  def onClickInGreenSliceView( self, interactorStyleTrackballCamera, event ):
    '''
    '''
    if not self.__manualSampling:
      return

    rasPos = Helper.onClickInSliceView( slicer.sliceWidgetGreen_interactorStyle, slicer.sliceWidgetGreen_sliceLogic )
    mrmlNode = self.__anatomicalTree.currentNode()
    self.mrmlManager().AddTreeNodeDistributionSamplePoint( self.__vtkId, rasPos );

    self.loadFromMRML( mrmlNode )
    self.__tabWidget.setCurrentIndex( 1 )

    Helper.Debug( ' GREEN RAS: ' + str( rasPos ) )



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


        self.__updating = 0

