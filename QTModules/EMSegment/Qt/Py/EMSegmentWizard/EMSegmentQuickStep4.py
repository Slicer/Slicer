from __main__ import qt, ctk, vtk
import PythonQt
import os
import math

from EMSegmentStep import *
from Helper import *

class EMSegmentQuickStep4( EMSegmentStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '4. Miscellaneous' )
    self.setDescription( 'Define miscellaneous parameters before performing segmentation' )

    self.__parent = super( EMSegmentQuickStep4, self )

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
    self.__roiWidget = None
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
    self.__roi.SetRadiusXYZ( 100, 100, 100 );
    self.__roi.Initialize( slicer.mrmlScene )
    self.__roi.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateMRMLFromROI )
    self.__roiWidget.setMRMLAnnotationROINode( self.__roi )
    self.__roiWidget.setDisplayClippingBox( 0 )


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

      if self.__roiWidget:
        self.updateROIFromMRML()

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

