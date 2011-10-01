from __main__ import qt, ctk

from ChangeTrackerStep import *
from Helper import *

import string

class ChangeTrackerReportROIStep( ChangeTrackerStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '5. ROI Analysis Results' )
    self.setDescription( '' )

    self.__vrDisplayNode = None
    self.__vrOpacityMap = None
    self.__vrLogic = slicer.modules.volumerendering.logic()

    self.__parent = super( ChangeTrackerReportROIStep, self )

  def createUserInterface( self ):
    '''
    '''
#    self.buttonBoxHints = self.ButtonBoxHidden

    print 'Creating user interface for last step!'
    self.__layout = self.__parent.createUserInterface()

    self.__metricsTabs = qt.QTabWidget()
    self.__layout.addRow(self.__metricsTabs)
    
  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )
    # check here that ROI is not empty and is within the baseline volume
    self.__parent.validationSucceeded(desiredBranchId)

  def onEntry(self, comingFrom, transitionType):
    Helper.Info('Report step: entering onEntry()')
    super(ChangeTrackerReportROIStep, self).onEntry(comingFrom, transitionType)

    pNode = self.parameterNode()
    Helper.Info('Report step: onEntry')
    # create the tabs
    self.__metricsTabs.clear()
    metrics = pNode.GetParameter('metrics')
    self.__metricTabsList = {}
    self.__metricsVolumes = {}

    print 'Metrics list: ', metrics
    metricsReports = string.split(pNode.GetParameter('resultReports'),',')
    metricsVolumesIDs = string.split(pNode.GetParameter('resultVolumes'),',')

    i = 0

    metricsList = string.split(metrics,',')
    print 'Total metrics: ',len(metricsList)

    print 'Metrics volumes ids = ', metricsVolumesIDs
    print 'Metrics = ', metricsList

    if len(metricsVolumesIDs) != len(metricsList):
      Helper.Error('Missing metric processing results!')

    for m in metricsList:
      print 'Adding tab for metric ',m
      metricWidget = qt.QWidget()
      metricLayout = qt.QFormLayout(metricWidget)
      textWidget = qt.QTextEdit()
      textWidget.setReadOnly(1)

      self.__metricsVolumes[m] = metricsVolumesIDs[i]
      currentVolume = Helper.getNodeByID(metricsVolumesIDs[i])

      textWidget.setText(currentVolume.GetDescription())
      metricLayout.addRow(textWidget)
      self.__metricsTabs.addTab(metricWidget, m)
      self.__metricTabsList[m] = textWidget
      print 'Finished preparing for ', m
      i = i+1

    self.__metricsTabs.connect("currentChanged(int)", self.onTabChanged)
    print 'Creating user interface for last step -- DONE!'


    # change the layout to Compare
    lm = slicer.app.layoutManager()
    lm.setLayout(12)
    lm.setLayoutNumberOfCompareViewRows(2)

    pNode = self.parameterNode()

    # use GetLayoutName() to identify the corresponding slice node and slice
    # composite node

    # find the compare nodes and initialize them as we wish
    sliceNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLSliceNode')
    sliceCompositeNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLSliceCompositeNode')

    # setup slice nodes
    for s in range(0,sliceNodes.GetNumberOfItems()):
      sNode = sliceNodes.GetItemAsObject(s)
      thisLayoutName = sNode.GetLayoutName()
      # TODO: check they should have the same layout name!
      if thisLayoutName.find('Compare') == 0:
        sNode.SetLayoutGrid(1,6)

    # setup slice composite nodes
    for s in range(0,sliceCompositeNodes.GetNumberOfItems()):
      scNode = sliceCompositeNodes.GetItemAsObject(s)
      thisLayoutName = scNode.GetLayoutName()
      if thisLayoutName == 'Compare1':
        scNode.SetBackgroundVolumeID(pNode.GetParameter('croppedBaselineVolumeID'))
        scNode.SetForegroundVolumeID('')
        scNode.SetLabelVolumeID('')
        scNode.SetLinkedControl(1)
      if thisLayoutName == 'Compare2':
        scNode.SetBackgroundVolumeID(pNode.GetParameter('croppedFollowupVolumeID'))
        scNode.SetForegroundVolumeID('')
        scNode.SetLabelVolumeID('')
        scNode.SetLinkedControl(1)


    '''
    setup for volume rendering
    '''
    if self.__vrDisplayNode == None:
      print 'Creating display node in the last step'
      self.__vrDisplayNode = self.__vrLogic.CreateVolumeRenderingDisplayNode()
      viewNode = slicer.util.getNode('vtkMRMLViewNode1')
      self.__vrDisplayNode.AddViewNodeID(viewNode.GetID())
      self.__vrDisplayNode.SetCurrentVolumeMapper(2)

    '''
    trigger volume rendering and label update
    '''
    self.onTabChanged(0)

    pNode.SetParameter('currentStep', self.stepid)

    Helper.Info('Report step: leaving onEntry()')

  def onTabChanged(self, index):

    metricName = self.__metricsTabs.tabText(index)
    print 'User selected metric ', metricName,
    print ' corresponding results volume: ', self.__metricsVolumes[metricName]
    sliceCompositeNodes = slicer.mrmlScene.GetNodesByClass('vtkMRMLSliceCompositeNode')

    for s in range(0,sliceCompositeNodes.GetNumberOfItems()):
      scNode = sliceCompositeNodes.GetItemAsObject(s)
      thisLayoutName = scNode.GetLayoutName()
      # TODO: check they should have the same layout name!
      if thisLayoutName == 'Compare1':
        scNode.SetLinkedControl(0)
        scNode.SetLabelVolumeID(self.__metricsVolumes[metricName])
        scNode.SetLinkedControl(1)
      if thisLayoutName == 'Compare2':
        scNode.SetLinkedControl(0)
        scNode.SetLabelVolumeID(self.__metricsVolumes[metricName])
        scNode.SetLinkedControl(1)
    
    self.showChangeMapVolumeRendering(self.__metricsVolumes[metricName])

  def showChangeMapVolumeRendering(self, labelID):
    '''
    volume render change detection results
    '''
    
    print 'Changes volume to render: ',labelID
    labelVolume = slicer.mrmlScene.GetNodeByID(labelID)

    self.__vrDisplayNode.SetAndObserveVolumeNodeID(labelID)
    self.__vrLogic.UpdateDisplayNodeFromVolumeNode(self.__vrDisplayNode, labelVolume)

    print 'Display node id:',self.__vrDisplayNode.GetID()

    vrOpacityMap = self.__vrDisplayNode.GetVolumePropertyNode().GetVolumeProperty().GetScalarOpacity()
    vrColorMap = self.__vrDisplayNode.GetVolumePropertyNode().GetVolumeProperty().GetRGBTransferFunction()
    
    # setup color transfer function once
    vrColorMap.RemoveAllPoints()
    vrColorMap.AddRGBPoint(11, 0, 0, 0) 
    vrColorMap.AddRGBPoint(12, 0.2, 0.8, 0.5) 
    vrColorMap.AddRGBPoint(12.1, 0, 0, 0) 
    vrColorMap.AddRGBPoint(13.9, 0, 0, 0) 
    vrColorMap.AddRGBPoint(14, 0.8, 0.2, 0.5) 
    vrColorMap.AddRGBPoint(15, 0, 0, 0) 

    vrOpacityMap.RemoveAllPoints()
    vrOpacityMap.AddPoint(0,0)
    vrOpacityMap.AddPoint(11,0)
    vrOpacityMap.AddPoint(12,1)
    vrOpacityMap.AddPoint(12.1,0)
    vrOpacityMap.AddPoint(13.9,0)
    vrOpacityMap.AddPoint(14,1)
    vrOpacityMap.AddPoint(15,0)

    self.__vrDisplayNode.VisibilityOn()
