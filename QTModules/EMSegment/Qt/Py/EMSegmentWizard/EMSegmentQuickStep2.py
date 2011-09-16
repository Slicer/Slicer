from __main__ import qt, ctk, slicer
import PythonQt

from Helper import *
from EMSegmentStep import *

class EMSegmentQuickStep2( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '2. Define Structures' )
    self.setDescription( 'Define a hierarchy of structures to be segmented.' )

    self.__parent = super( EMSegmentQuickStep2, self )
    self.__layout = None
    self.__colorTableComboBox = None
    self.__anatomicalTree = None

    self.__numberOfStructures = 0

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


  def reset( self ):
    '''
    '''
    self.__numberOfStructures = 0

  def disableNumberOfStructures( self ):
    '''
    '''
    self.__numberOfStructures = 1

  def onEntry( self, comingFrom, transitionType ):
    '''
    '''
    self.__parent.onEntry( comingFrom, transitionType )

    if comingFrom.id() == Helper.GetNthStepId( 7 ):
      return

    if self.__numberOfStructures == 0:

      self.__numberOfStructures = qt.QInputDialog.getInt( self, 'Number of Structures', 'Please specify how many different structures are in the input image(s).', 3, 1 )

      # no harm for division by zero since the input dialog prevents zero
      probability = 1.0 / self.__numberOfStructures

      for i in range( self.__numberOfStructures ):

        currentId = self.mrmlManager().AddTreeNode( self.mrmlManager().GetTreeRootNodeID() )
        if i == 0:
          self.mrmlManager().SetTreeNodeName( currentId, 'Background' )
          self.mrmlManager().SetTreeNodeIntensityLabel( currentId, 0 )
        elif i > 0:
          self.mrmlManager().SetTreeNodeName( currentId, 'Tissue' + str( i ) )
          self.mrmlManager().SetTreeNodeIntensityLabel( currentId, i )

        self.mrmlManager().SetTreeNodeClassProbability( currentId, probability )
        self.mrmlManager().SetTreeNodeSpatialPriorWeight( currentId, 0.0 )

    if self.__anatomicalTree:
      self.__anatomicalTree.setMRMLManager( self.mrmlManager() )
      self.__anatomicalTree.updateWidgetFromMRML()


  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )

    self.__parent.validationSucceeded( desiredBranchId )
