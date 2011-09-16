from __main__ import qt, ctk, vtk
import PythonQt
import os
import math

from EMSegmentStep import *
from Helper import *
from LabelStatistics import LabelStatisticsWidget

class EMSegmentLabelStatisticsStep( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( 'Segmentation finished' )
    self.setDescription( 'View after-segmentation statistics.' )

    self.__parent = super( EMSegmentLabelStatisticsStep, self )

    self.__updating = 0;


  def createUserInterface( self ):
    '''
    '''
    self.__layout = self.__parent.createUserInterface()

    infoLabel = qt.QLabel( 'The segmentation finished!\n\nThe results are displayed as labelMaps in the slice viewers.\n\n' )
    infoLabel.setFont( self.__parent.getBoldFont() )
    self.__layout.addWidget( infoLabel )

    self.__top = qt.QGroupBox()
    self.__top.setTitle( 'Statistics' )
    self.__top.toolTip = 'This table shows label statistics for the segmentation results.'
    self.__topLayout = qt.QVBoxLayout( self.__top )

    self.__layout.addWidget( self.__top )

    # deactivate next button since it is the last step
    if slicer.modules.emsegmentsimplemode:
      self.buttonBoxHints = self.ButtonBoxHidden
    else:
      self.buttonBoxHints = self.NextButtonDisabled

    # the volume node
    workingDataNode = self.mrmlManager().GetWorkingDataNode()

    if workingDataNode:
      # set flags in the mrml nodes
      workingDataNode.SetAlignedTargetNodeIsValid( 1 )
      workingDataNode.SetAlignedAtlasNodeIsValid( 1 )

      # show preprocessing output in sliceViews
      volumeCollection = workingDataNode.GetInputTargetNode()
      if volumeCollection:
        outputNode = volumeCollection.GetNthVolumeNode( 0 )

    labelStatisticsWidget = LabelStatisticsWidget( self.__top )
    labelStatisticsWidget.setup()
    labelStatisticsWidget.labelNode = self.mrmlManager().GetOutputVolumeNode()
    labelStatisticsWidget.grayscaleNode = outputNode

    labelStatisticsWidget.applyButton.visible = False
    labelStatisticsWidget.labelSelectorFrame.visible = False
    labelStatisticsWidget.grayscaleSelectorFrame.visible = False
    labelStatisticsWidget.saveButton.visible = False
    labelStatisticsWidget.grayscaleSelector.visible = False
    labelStatisticsWidget.labelSelector.visible = False

    labelStatisticsWidget.onApply()

    if slicer.modules.emsegmentsimplemode:
      infoLabel2 = qt.QLabel( '\n\nIt is now possible to reset the module.' )
      #infoLabel2.setFont( self.__parent.getBoldFont() )
      self.__layout.addWidget( infoLabel2 )

      resetButton = qt.QPushButton( 'Reset Module' )
      resetButton.connect( 'clicked()', self.onResetButton )
      self.__layout.addWidget( resetButton )

  def onResetButton( self ):
    '''
    '''
    self.workflow().goBackward() # 9
    self.workflow().goBackward() # 8
    self.workflow().goBackward() # 7 
    self.workflow().goBackward() # 6
    self.workflow().goBackward() # 5
    self.workflow().goBackward() # 4
    self.workflow().goBackward() # 3
    self.workflow().goBackward( 'SimpleMode' ) # 2



    #self.__layout.addWidget( labelStatisticsWidget )


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

      self.__updating = 0



