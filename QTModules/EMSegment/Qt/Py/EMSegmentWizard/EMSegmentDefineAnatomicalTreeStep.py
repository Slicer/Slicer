from __main__ import qt, ctk, slicer
import PythonQt

from EMSegmentStep import *

class EMSegmentDefineAnatomicalTreeStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '3. Define Anatomical Tree' )
    self.setDescription( 'Define a hierarchy of structures.' )

    self.__parent = super( EMSegmentDefineAnatomicalTreeStep, self )
    self.__layout = None
    self.__colorTableComboBox = None
    self.__anatomicalTree = None

  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    # the colorpicker combobox
    self.__colorTableComboBox = slicer.qMRMLColorTableComboBox()
    self.__colorTableComboBox.setMRMLScene( slicer.mrmlScene )
    self.__colorTableComboBox.toolTip = 'The label colors are defined using this look-up table.'

    infoLabel = qt.QLabel( "Choose the look-up table for label colorization:" )
    self.__layout.addWidget( infoLabel )

    # get current color node from the mrmlManager
    currentColorNodeID = self.mrmlManager().GetColorNodeID()
    currentColorNode = slicer.mrmlScene.GetNodeByID( currentColorNodeID )
    if currentColorNode:
      self.__colorTableComboBox.setCurrentNode( currentColorNode )

    self.__colorTableComboBox.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onColorNodeChanged )
    self.__layout.addWidget( self.__colorTableComboBox )

    # add empty row
    self.__layout.addRow( "", qt.QWidget() )
    # add empty row
    self.__layout.addRow( "", qt.QWidget() )
    # add empty row
    self.__layout.addRow( "", qt.QWidget() )

    # the anatomical tree
    anatomicalTreeGroupBox = qt.QGroupBox()
    anatomicalTreeGroupBox.setTitle( 'Anatomical Tree' )
    self.__layout.addWidget( anatomicalTreeGroupBox )

    anatomicalTreeGroupBoxLayout = qt.QFormLayout( anatomicalTreeGroupBox )

    self.__anatomicalTree = PythonQt.qSlicerEMSegmentModuleWidgets.qSlicerEMSegmentAnatomicalTreeWidget()
    self.__anatomicalTree.structureNameEditable = True
    self.__anatomicalTree.labelColumnVisible = True
    self.__anatomicalTree.addDeleteSubclassesEnabled = True
    self.__anatomicalTree.toolTip = 'Please configure a hierarchy of structures for the input datasets.'
    self.__anatomicalTree.setSizePolicy( qt.QSizePolicy.MinimumExpanding, qt.QSizePolicy.MinimumExpanding )
    anatomicalTreeGroupBoxLayout.addWidget( self.__anatomicalTree )

  def onColorNodeChanged( self ):
    '''
    '''
    # use the selected colorNodeID
    self.mrmlManager().SetColorNodeID( self.__colorTableComboBox.currentNodeId )

    # .. propagate to widget
    self.__anatomicalTree.updateWidgetFromMRML()


  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    if self.__anatomicalTree:
      self.__anatomicalTree.setMRMLManager( self.mrmlManager() )
      self.__anatomicalTree.updateWidgetFromMRML()


  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )
