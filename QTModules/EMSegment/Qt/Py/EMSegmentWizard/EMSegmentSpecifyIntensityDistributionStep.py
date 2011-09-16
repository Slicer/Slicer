from __main__ import qt, ctk
import PythonQt

from EMSegmentStep import *
from Helper import *

class EMSegmentSpecifyIntensityDistributionStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '7. Specify Intensity Distributions' )
    self.setDescription( 'Define the intensity distribution for each anatomical structure.' )

    self.__parent = super( EMSegmentSpecifyIntensityDistributionStep, self )
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

    slicer.sliceWidgetRed_interactorStyle.AddObserver( vtk.vtkCommand.LeftButtonReleaseEvent, self.onClickInRedSliceView )
    slicer.sliceWidgetYellow_interactorStyle.AddObserver( vtk.vtkCommand.LeftButtonReleaseEvent, self.onClickInYellowSliceView )
    slicer.sliceWidgetGreen_interactorStyle.AddObserver( vtk.vtkCommand.LeftButtonReleaseEvent, self.onClickInGreenSliceView )

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    # the anatomical tree
    anatomicalTreeGroupBox = qt.QGroupBox()
    anatomicalTreeGroupBox.setTitle( 'Anatomical Tree' )
    self.__layout.addWidget( anatomicalTreeGroupBox )

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

    self.__tabWidget = qt.QTabWidget()
    self.__layout.addWidget( self.__tabWidget )

    #
    # intensityDistributionPage
    #
    intensityDistributionPage = qt.QWidget()
    intensityDistributionPageLayout = qt.QFormLayout( intensityDistributionPage )

    self.__classLabel = qt.QLabel( "XX" )
    intensityDistributionPageLayout.addRow( "Class:", self.__classLabel )

    self.__specificationComboBox = qt.QComboBox()
    self.__specificationComboBox.addItems( Helper.GetSpecificationTypes() )
    #self.__specificationComboBox.model().item( 2 ).setSelectable( False )
    #self.__specificationComboBox.model().item( 2 ).setEnabled( False )
    self.__specificationComboBox.toolTip = 'The intensity distribution can be specified manually or through manual sampling.'
    intensityDistributionPageLayout.addRow( "Specification:", self.__specificationComboBox )
    self.__specificationComboBox.connect( 'currentIndexChanged(int)', self.propagateToMRML )

    self.__meanMatrixWidget = ctk.ctkMatrixWidget()
    self.__meanMatrixWidget.columnCount = 1
    self.__meanMatrixWidget.rowCount = 1
    self.__meanMatrixWidget.decimals = 4
    self.__meanMatrixWidget.minimum = 0
    self.__meanMatrixWidget.maximum = 1000000
    self.__meanMatrixWidget.toolTip = 'The mean intensity value for this structure.'
    intensityDistributionPageLayout.addRow( "Mean:", self.__meanMatrixWidget )
    self.__meanMatrixWidget.connect( 'matrixChanged()', self.propagateToMRML )

    self.__logCovarianceMatrixWidget = ctk.ctkMatrixWidget()
    self.__logCovarianceMatrixWidget.columnCount = 1
    self.__logCovarianceMatrixWidget.rowCount = 1
    self.__logCovarianceMatrixWidget.decimals = 4
    self.__logCovarianceMatrixWidget.minimum = 0
    self.__logCovarianceMatrixWidget.maximum = 1000000
    self.__logCovarianceMatrixWidget.toolTip = 'The log covariance for this structure.'
    intensityDistributionPageLayout.addRow( "Log Covariance:", self.__logCovarianceMatrixWidget )
    self.__logCovarianceMatrixWidget.connect( 'matrixChanged()', self.propagateToMRML )

    self.__resetDistributionButton = qt.QPushButton()
    self.__resetDistributionButton.text = "Reset Distribution"
    self.__resetDistributionButton.toolTip = 'Reset the distribution to the old value.'
    intensityDistributionPageLayout.addRow( self.__resetDistributionButton )
    self.__resetDistributionButton.connect( 'clicked()', self.resetDistribution )

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

    self.__tabWidget.addTab( intensityDistributionPage, "Intensity Distribution" )
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

        self.__classLabel.setText( mrmlNode.GetName() )
        self.__classLabel2.setText( "Class: " + mrmlNode.GetName() )
        self.__specificationComboBox.setCurrentIndex( self.mrmlManager().GetTreeNodeDistributionSpecificationMethod( vtkId ) )

        if self.__specificationComboBox.currentIndex == 1:
          # manual sampling, enable the manual sampling tab
          self.__tabWidget.setTabEnabled( 1, True )
          self.setupManualSampleTable( mrmlNode )



        self.__meanMatrixWidget.columnCount = numberOfVolumes
        self.__meanMatrixWidget.rowCount = 1
        # only editable, if specification is Manual
        self.__meanMatrixWidget.editable = ( self.__specificationComboBox.currentIndex == 0 )
        # fill values
        for c in range( numberOfVolumes ):

          value = self.mrmlManager().GetTreeNodeDistributionMeanWithCorrection( vtkId, c )
          Helper.Debug( 'Mean:' + str( value ) )
          self.__meanMatrixWidget.setValue( 0, c, value )

        self.__logCovarianceMatrixWidget.columnCount = numberOfVolumes
        self.__logCovarianceMatrixWidget.rowCount = numberOfVolumes
        # only editable, if specification is Manual
        self.__logCovarianceMatrixWidget.editable = ( self.__specificationComboBox.currentIndex == 0 )
        # fill values
        for r in range( numberOfVolumes ):
          for c in range( numberOfVolumes ):
            value = self.mrmlManager().GetTreeNodeDistributionLogCovarianceWithCorrection( vtkId, r, c )
            self.__logCovarianceMatrixWidget.setValue( r, c, value )

      self.__updating = 0

  def setupManualSampleTable( self, mrmlNode ):
    '''
    '''
    vtkId = self.mrmlManager().MapMRMLNodeIDToVTKNodeID( mrmlNode.GetID() )

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
    if self.__classLabel:
      self.__classLabel.setText( '' )

    if self.__classLabel2:
      self.__classLabel2.setText( '' )

    if self.__specificationComboBox:
      self.__specificationComboBox.setEnabled( enabled )
      self.__specificationComboBox.setCurrentIndex( 0 )

    if self.__meanMatrixWidget:
      self.__meanMatrixWidget.setEnabled( enabled )
      self.__meanMatrixWidget.columnCount = 1
      self.__meanMatrixWidget.rowCount = 1
      self.__meanMatrixWidget.setValue( 0, 0, 0 )

    if self.__logCovarianceMatrixWidget:
      self.__logCovarianceMatrixWidget.setEnabled( enabled )
      self.__logCovarianceMatrixWidget.columnCount = 1
      self.__logCovarianceMatrixWidget.rowCount = 1
      self.__logCovarianceMatrixWidget.setValue( 0, 0, 0 )

    if self.__resetDistributionButton:
      self.__resetDistributionButton.setEnabled( enabled )

    if self.__tabWidget:
      self.__tabWidget.setEnabled( enabled )
      self.__tabWidget.setTabEnabled( 1, False )

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
