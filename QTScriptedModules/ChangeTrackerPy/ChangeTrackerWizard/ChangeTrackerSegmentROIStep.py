from __main__ import qt, ctk

from ChangeTrackerStep import *
from Helper import *

import string

'''
TODO:
  add advanced option to specify segmentation
'''

class ChangeTrackerSegmentROIStep( ChangeTrackerStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '3. Segment the analyzed structure' )
    self.setDescription( 'Segment the structure in the selected ROI.' )

    self.__parent = super( ChangeTrackerSegmentROIStep, self )
    
    self.__vrDisplayNode = None
    self.__threshold = [ -1, -1 ]
       
    # initialize VR stuff
    self.__vrLogic = slicer.modules.volumerendering.logic()
    self.__vrOpacityMap = None

    self.__roiSegmentationNode = None
    self.__roiVolume = None

  def createUserInterface( self ):
    '''
    '''
    print 'SegmentROI create interface'
#    self.buttonBoxHints = self.ButtonBoxHidden

    self.__layout = self.__parent.createUserInterface()

    self.__basicFrame = ctk.ctkCollapsibleButton()
    self.__basicFrame.text = "Basic settings"
    self.__basicFrame.collapsed = 0
    basicFrameLayout = qt.QFormLayout(self.__basicFrame)
    self.__layout.addRow(self.__basicFrame)

    self.__advancedFrame = ctk.ctkCollapsibleButton()
    self.__advancedFrame.text = "Advanced settings"
    self.__advancedFrame.collapsed = 1
    advFrameLayout = qt.QFormLayout(self.__advancedFrame)
    self.__layout.addRow(self.__advancedFrame)

    threshLabel = qt.QLabel('Choose threshold:')
    self.__threshRange = slicer.qMRMLRangeWidget()
    self.__threshRange.decimals = 0
    self.__threshRange.singleStep = 1

    self.__useThresholdsCheck = qt.QCheckBox()
    self.__useThresholdsCheck.setEnabled(0)
    threshCheckLabel = qt.QLabel('Use thresholds for segmentation')

    roiLabel = qt.QLabel( 'Select segmentation:' )
    self.__roiLabelSelector = slicer.qMRMLNodeComboBox()
    self.__roiLabelSelector.nodeTypes = ( 'vtkMRMLScalarVolumeNode', '' )
    self.__roiLabelSelector.addAttribute('vtkMRMLScalarVolumeNode','LabelMap','1')
    self.__roiLabelSelector.toolTip = "Choose the ROI segmentation"
    self.__roiLabelSelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__roiLabelSelector.addEnabled = 0
    self.__roiLabelSelector.setMRMLScene(slicer.mrmlScene)

    basicFrameLayout.addRow(threshLabel, self.__threshRange)
    advFrameLayout.addRow(threshCheckLabel, self.__useThresholdsCheck)
    advFrameLayout.addRow( roiLabel, self.__roiLabelSelector )

    self.__threshRange.connect('valuesChanged(double,double)', self.onThresholdChanged)
    self.__useThresholdsCheck.connect('stateChanged(int)', self.onThresholdsCheckChanged)

  def onThresholdsCheckChanged(self):
    if self.__useThresholdsCheck.isChecked():
      self.__roiLabelSelector.setEnabled(0)
      self.__threshRange.setEnabled(1)
    else:
      self.__roiLabelSelector.setEnabled(1)
      self.__threshRange.setEnabled(0)
    

  def onThresholdChanged(self): 
    
    if self.__vrOpacityMap == None:
      return
    

    range0 = self.__threshRange.minimumValue
    range1 = self.__threshRange.maximumValue

    self.__vrOpacityMap.RemoveAllPoints()
    self.__vrOpacityMap.AddPoint(0,0)
    self.__vrOpacityMap.AddPoint(0,0)
    self.__vrOpacityMap.AddPoint(range0-1,0)
    self.__vrOpacityMap.AddPoint(range0,1)
    self.__vrOpacityMap.AddPoint(range1,1)
    self.__vrOpacityMap.AddPoint(range1+1,0)

    # update the label volume accordingly
    thresh = vtk.vtkImageThreshold()
    thresh.SetInput(self.__roiVolume.GetImageData())
    thresh.ThresholdBetween(range0, range1)
    thresh.SetInValue(10)
    thresh.SetOutValue(0)
    thresh.ReplaceOutOn()
    thresh.ReplaceInOn()
    thresh.Update()

    self.__roiSegmentationNode.SetAndObserveImageData(thresh.GetOutput())

  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )
    self.__parent.validationSucceeded(desiredBranchId)

  def onExit(self, goingTo, transitionType):
    self.__vrDisplayNode.VisibilityOff()

    pNode = self.parameterNode()
    pNode.SetParameter('thresholdRange', str(self.__threshRange.minimumValue)+','+str(self.__threshRange.maximumValue))


    super(ChangeTrackerSegmentROIStep, self).onExit(goingTo, transitionType)

  def onEntry(self, comingFrom, transitionType):
    '''
    Resample the baseline volume using ROI

    TODO: if coming from the next step, do not resample!

    TODO: this should go to onExit() in the previous step!
    '''
    super(ChangeTrackerSegmentROIStep, self).onEntry(comingFrom, transitionType)

    pNode = self.parameterNode()
    self.updateWidgetFromParameters(pNode)

    self.onThresholdsCheckChanged()

    Helper.SetBgFgVolumes(pNode.GetParameter('croppedBaselineVolumeID'),'')

    # TODO: initialize volume selectors, fit ROI to slice viewers, create
    # label volume, initialize the threshold, initialize volume rendering ?

    roiVolume = Helper.getNodeByID(pNode.GetParameter('croppedBaselineVolumeID'))
    self.__roiVolume = roiVolume
    self.__roiSegmentationNode = Helper.getNodeByID(pNode.GetParameter('croppedBaselineVolumeSegmentationID'))

    if self.__vrDisplayNode == None:
      self.__vrDisplayNode = self.__vrLogic.CreateVolumeRenderingDisplayNode()
      viewNode = slicer.util.getNode('vtkMRMLViewNode1')
      self.__vrDisplayNode.AddViewNodeID(viewNode.GetID())
      self.__vrDisplayNode.SetCurrentVolumeMapper(2)

    if self.__useThresholds:
      self.__vrDisplayNode.SetAndObserveVolumeNodeID(roiVolume.GetID())
      self.__vrLogic.UpdateDisplayNodeFromVolumeNode(self.__vrDisplayNode, roiVolume)
      self.__vrOpacityMap = self.__vrDisplayNode.GetVolumePropertyNode().GetVolumeProperty().GetScalarOpacity()
      vrColorMap = self.__vrDisplayNode.GetVolumePropertyNode().GetVolumeProperty().GetRGBTransferFunction()
    
    # setup color transfer function once
    
    baselineROIVolume = Helper.getNodeByID(pNode.GetParameter('croppedBaselineVolumeID'))
    baselineROIRange = baselineROIVolume.GetImageData().GetScalarRange()

    vrColorMap.RemoveAllPoints()
    vrColorMap.AddRGBPoint(0, 0, 0, 0) 
    vrColorMap.AddRGBPoint(baselineROIRange[0]-1, 0, 0, 0) 
    vrColorMap.AddRGBPoint(baselineROIRange[0], 0.8, 0.8, 0) 
    vrColorMap.AddRGBPoint(baselineROIRange[1], 0.8, 0.8, 0) 
    vrColorMap.AddRGBPoint(baselineROIRange[1]+1, 0, 0, 0) 

    self.__vrDisplayNode.VisibilityOn()

    threshRange = [self.__threshRange.minimumValue, self.__threshRange.maximumValue]
    self.__vrOpacityMap.RemoveAllPoints()
    self.__vrOpacityMap.AddPoint(0,0)
    self.__vrOpacityMap.AddPoint(0,0)
    self.__vrOpacityMap.AddPoint(threshRange[0]-1,0)
    self.__vrOpacityMap.AddPoint(threshRange[0],1)
    self.__vrOpacityMap.AddPoint(threshRange[1],1)
    self.__vrOpacityMap.AddPoint(threshRange[1]+1,0)

    labelsColorNode = slicer.modules.colors.logic().GetColorTableNodeID(10)
    self.__roiSegmentationNode.GetDisplayNode().SetAndObserveColorNodeID(labelsColorNode)

    Helper.SetLabelVolume(self.__roiSegmentationNode.GetID())

    self.onThresholdChanged()
    
    pNode.SetParameter('currentStep', self.stepid)

  def updateWidgetFromParameters(self, pNode):
  
    baselineROIVolume = Helper.getNodeByID(pNode.GetParameter('croppedBaselineVolumeID'))
    baselineROIRange = baselineROIVolume.GetImageData().GetScalarRange()
    self.__threshRange.minimum = baselineROIRange[0]
    self.__threshRange.minimum = baselineROIRange[1]

    if pNode.GetParameter('useSegmentationThresholds') == 'True':
      self.__useThresholds = True
      self.__useThresholdsCheck.setChecked(1)

      thresholdRange = pNode.GetParameter('thresholdRange')
      if thresholdRange != '':
        rangeArray = string.split(thresholdRange, ',')
        self.__threshRange.minimumValue = float(rangeArray[0])
        self.__threshRange.maximumValue = float(rangeArray[1])
      else:
         Helper.Error('Unexpected parameter values! Error code CT-S03-TNA. Please report')
    else:
      self.__useThresholdsCheck.setChecked(0)
      self.__useThresholds = False

    segmentationID = pNode.GetParameter('croppedBaselineVolumeSegmentationID')
    if segmentationID != '':
      self.__roiLabelSelector.setCurrentNode(Helper.getNodeByID(segmentationID))
    else:
      Helper.Error('Unexpected parameter values! Error CT-S03-SNA. Please report')
    self.__roiSegmentationNode = Helper.getNodeByID(segmentationID)
