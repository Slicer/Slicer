from __main__ import qt, ctk

from ChangeTrackerStep import *
from Helper import *

class ChangeTrackerRegistrationStep( ChangeTrackerStep ) :

  def __init__( self, stepid ):
    self.initialize( stepid )
    self.setName( '4. Registration' )
    self.setDescription( 'Align follow-up scan with the baseline.' )

    self.__parent = super( ChangeTrackerRegistrationStep, self )

    self.__followupTransform = None

  def createUserInterface( self ):
    '''
    '''
#    self.buttonBoxHints = self.ButtonBoxHidden

    self.__layout = self.__parent.createUserInterface()

    # add registration button
    self.__registrationButton = qt.QPushButton('Run registration')
    self.__registrationStatus = qt.QLabel('Register scans')

    self.__layout.addRow(self.__registrationStatus, self.__registrationButton)

    self.__registrationButton.connect('clicked()', self.onRegistrationRequest)

  def validate( self, desiredBranchId ):
    '''
    '''
    self.__parent.validate( desiredBranchId )
    pNode = self.parameterNode()
    followupVolume = slicer.mrmlScene.GetNodeByID(pNode.GetParameter('followupVolumeID'))
    followupTransform = followupVolume.GetTransformNodeID()
    if followupTransform == None:
      self.__parent.validationFailed(desiredBranchId)
    self.__parent.validationSucceeded(desiredBranchId)

  # def onEntry(self, comingFrom, transitionType):

  def onRegistrationRequest(self):

    # rigidly register followup to baseline
    # TODO: do this in a separate step and allow manual adjustment?
    # TODO: add progress reporting (BRAINSfit does not report progress though)
    pNode = self.parameterNode()
    baselineVolumeID = pNode.GetParameter('baselineVolumeID')
    followupVolumeID = pNode.GetParameter('followupVolumeID')
    self.__followupTransform = slicer.mrmlScene.CreateNodeByClass('vtkMRMLLinearTransformNode')
    slicer.mrmlScene.AddNode(self.__followupTransform)

    parameters = {}
    parameters["fixedVolume"] = baselineVolumeID
    parameters["movingVolume"] = followupVolumeID
    parameters["initializeTransformMode"] = "useMomentsAlign"
    parameters["useRigid"] = True
    parameters["useScaleVersor3D"] = True
    parameters["useScaleSkewVersor3D"] = True
    parameters["useAffine"] = True
    parameters["linearTransform"] = self.__followupTransform.GetID()

    self.__cliNode = None
    self.__cliNode = slicer.cli.run(slicer.modules.brainsfit, self.__cliNode, parameters)

    self.__cliObserverTag = self.__cliNode.AddObserver('ModifiedEvent', self.processRegistrationCompletion)
    self.__registrationStatus.setText('Wait ...')
    self.__registrationButton.setEnabled(0)


  def processRegistrationCompletion(self, node, event):
    status = node.GetStatusString()
    self.__registrationStatus.setText('Registration '+status)
    if status == 'Completed':
      self.__registrationButton.setEnabled(1)
  
      pNode = self.parameterNode()
      followupNode = slicer.mrmlScene.GetNodeByID(pNode.GetParameter('followupVolumeID'))
      followupNode.SetAndObserveTransformNodeID(self.__followupTransform.GetID())
      
      Helper.SetBgFgVolumes(pNode.GetParameter('baselineVolumeID'),pNode.GetParameter('followupVolumeID'))

      pNode.SetParameter('followupTransformID', self.__followupTransform.GetID())
