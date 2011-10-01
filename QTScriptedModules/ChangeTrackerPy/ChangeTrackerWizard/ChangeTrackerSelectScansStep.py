from __main__ import qt, ctk

from ChangeTrackerStep import *
from Helper import *

class ChangeTrackerSelectScansStep( ChangeTrackerStep ) :

  def __init__( self, stepid ):
    print 'Select scans: init'
    self.initialize( stepid )
    self.setName( '1. Select input scans' )
    self.setDescription( 'Select the baseline and follow-up scans to be compared.' )

    self.__parent = super( ChangeTrackerSelectScansStep, self )

  def createUserInterface( self ):
    '''
    '''
    print 'SelectScans step: createInterface'
    # TODO: might make sense to hide the button for the last step at this
    # point, but the widget does not have such option
    self.__layout = self.__parent.createUserInterface()
   
    baselineScanLabel = qt.QLabel( 'Baseline scan:' )
    self.__baselineVolumeSelector = slicer.qMRMLNodeComboBox()
    self.__baselineVolumeSelector.toolTip = "Choose the baseline scan"
    self.__baselineVolumeSelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__baselineVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.__baselineVolumeSelector.addEnabled = 0

    followupScanLabel = qt.QLabel( 'Followup scan:' )
    self.__followupVolumeSelector = slicer.qMRMLNodeComboBox()
    self.__followupVolumeSelector.toolTip = "Choose the followup scan"
    self.__followupVolumeSelector.nodeTypes = ['vtkMRMLScalarVolumeNode']
    self.__followupVolumeSelector.setMRMLScene(slicer.mrmlScene)
    self.__followupVolumeSelector.addEnabled = 0
   
    loadDataButton = qt.QPushButton('Load test data')
    self.__layout.addRow(loadDataButton)
    loadDataButton.connect('clicked()', self.loadData)

    self.__layout.addRow( baselineScanLabel, self.__baselineVolumeSelector )
    self.__layout.addRow( followupScanLabel, self.__followupVolumeSelector )

    self.updateWidgetFromParameters(self.parameterNode())

  def loadData(self):
    vl = slicer.modules.volumes.logic()
    vl.AddArchetypeVolume('http://www.slicer.org/slicerWiki/images/5/59/RegLib_C01_1.nrrd', 'Meningioma1', 0)
    vl.AddArchetypeVolume('http://www.slicer.org/slicerWiki/images/e/e3/RegLib_C01_2.nrrd', 'Meningioma2', 0)

  def validate( self, desiredBranchId ):
    '''
    '''
    print 'SelectScans step: validate'
    self.__parent.validate( desiredBranchId )

    # check here that the selectors are not empty
    baseline = self.__baselineVolumeSelector.currentNode()
    followup = self.__followupVolumeSelector.currentNode()

    if baseline != None and followup != None:
      baselineID = baseline.GetID()
      followupID = followup.GetID()
      if baselineID != '' and followupID != '' and baselineID != followupID:
    
        pNode = self.parameterNode()
        pNode.SetParameter('baselineVolumeID', baselineID)
        pNode.SetParameter('followupVolumeID', followupID)
        
        self.__parent.validationSucceeded(desiredBranchId)
      else:
        self.__parent.validationFailed(desiredBranchId, 'Error','Please select distinctive baseline and followup volumes!')
    else:
      self.__parent.validationFailed(desiredBranchId, 'Error','Please select both baseline and followup volumes!')

  def onEntry(self, comingFrom, transitionType):
    print 'SelectScans step: onEntry'
    super(ChangeTrackerSelectScansStep, self).onEntry(comingFrom, transitionType)
    self.updateWidgetFromParameters(self.parameterNode())
    pNode = self.parameterNode()
    pNode.SetParameter('currentStep', self.stepid)

  def onExit(self, goingTo, transitionType):
    print 'SelectScans step: onExit'
    self.doStepProcessing()
    super(ChangeTrackerSelectScansStep, self).onExit(goingTo, transitionType) 

  def updateWidgetFromParameters(self, parameterNode):
    baselineVolumeID = parameterNode.GetParameter('baselineVolumeID')
    followupVolumeID = parameterNode.GetParameter('followupVolumeID')
    if baselineVolumeID != None:
      self.__baselineVolumeSelector.setCurrentNode(Helper.getNodeByID(baselineVolumeID))
    if followupVolumeID != None:
      self.__followupVolumeSelector.setCurrentNode(Helper.getNodeByID(followupVolumeID))


  def doStepProcessing(self):
    # calculate the transform to align the ROI in the next step with the
    # baseline volume
    pNode = self.parameterNode()

    baselineVolume = Helper.getNodeByID(pNode.GetParameter('baselineVolumeID'))
    roiTransformID = pNode.GetParameter('roiTransformID')
    roiTransformNode = None
    
    if roiTransformID != '':
      roiTransformNode = Helper.getNodeByID(roiTransformID)
    else:
      roiTransformNode = slicer.mrmlScene.CreateNodeByClass('vtkMRMLLinearTransformNode')
      slicer.mrmlScene.AddNode(roiTransformNode)
      pNode.SetParameter('roiTransformID', roiTransformNode.GetID())
      print 'Added transform node: ', pNode.GetParameter('roiTransformID')

    dm = vtk.vtkMatrix4x4()
    baselineVolume.GetIJKToRASDirectionMatrix(dm)
    dm.SetElement(0,3,0)
    dm.SetElement(1,3,0)
    dm.SetElement(2,3,0)
    dm.SetElement(0,0,abs(dm.GetElement(0,0)))
    dm.SetElement(1,1,abs(dm.GetElement(1,1)))
    dm.SetElement(2,2,abs(dm.GetElement(2,2)))
    roiTransformNode.SetAndObserveMatrixTransformToParent(dm)
