#include "qSlicerTractographyFiducialSeedingModuleWidget.h"
#include "ui_qSlicerTractographyFiducialSeedingModule.h"

#include "vtkSlicerTractographyFiducialSeedingLogic.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLTractographyFiducialSeedingNode.h"


//-----------------------------------------------------------------------------
class qSlicerTractographyFiducialSeedingModuleWidgetPrivate: 
  public qCTKPrivate<qSlicerTractographyFiducialSeedingModuleWidget>,
  public Ui_qSlicerTractographyFiducialSeedingModule
{
};

//-----------------------------------------------------------------------------
qSlicerTractographyFiducialSeedingModuleWidget::qSlicerTractographyFiducialSeedingModuleWidget(QWidget *parent)
{
  this->TractographyFiducialSeedingNode = NULL;
  this->TransformableNode = NULL;
  this->DiffusionTensorVolumeNode = NULL;

}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  qSlicerWidget::setMRMLScene(scene);

  // find parameters node or create it if there is no one in the scene
  if (this->TractographyFiducialSeedingNode == NULL)
  {
    vtkMRMLTractographyFiducialSeedingNode *tnode = NULL;
    vtkMRMLNode *node = scene->GetNthNodeByClass(0, "vtkMRMLTractographyFiducialSeedingNode");
    if (node == NULL)
    {
      tnode = vtkMRMLTractographyFiducialSeedingNode::New();
      scene->AddNode(tnode);
      tnode->Delete();
    }
    else {
      tnode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(node);
    }
    this->setTractographyFiducialSeedingNode(tnode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setup()
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);
  d->setupUi(this);


  QObject::connect(d->ParameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, 
                                             SLOT(setTractographyFiducialSeedingNode(vtkMRMLNode*)));

  QObject::connect(d->DTINodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, 
                                       SLOT(setDiffusionTensorVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->FiducialNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, 
                                            SLOT(setTransformableNode(vtkMRMLNode*)));

  QObject::connect(d->FiberNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, 
                                         SLOT(setFiberBundleNode(vtkMRMLNode*)));

  //QObject::connect(d->StoppingCurvatureSpinBoxLabel,
  //                 SIGNAL(sliderMoved(double)),
  //                 SLOT(onParameterChanged(double)));

  QObject::connect(d->StoppingCurvatureSpinBoxLabel, 
                SIGNAL(valueChanged(double)),
                SLOT(setStoppingCurvature(double)));

  QObject::connect(d->StoppingCriteriaComboBox, 
                SIGNAL(currentIndexChanged(int)),
                SLOT(setStoppingCriteria(int)));

  QObject::connect(d->StoppingValueSpinBoxLabel, 
                SIGNAL(valueChanged(double)),
                SLOT(setStoppingValue(double)));

  QObject::connect(d->IntegrationStepSpinBoxLabel, 
                SIGNAL(valueChanged(double)),
                SLOT(setIntegrationStep(double)));

  QObject::connect(d->MinimumPathSpinBoxLabel, 
                SIGNAL(valueChanged(double)),
                SLOT(setMinimumPath(double)));

  QObject::connect(d->FiducialRegionSpinBoxLabel, 
                SIGNAL(valueChanged(double)),
                SLOT(setFiducialRegion(double)));

  QObject::connect(d->FiducialStepSpinBoxLabel, 
                SIGNAL(valueChanged(double)),
                SLOT(setFiducialRegionStep(double)));

  QObject::connect(d->DisplayTracksComboBox, 
                SIGNAL(currentIndexChanged(int)),
                SLOT(setTrackDisplayMode(int)));

  QObject::connect(d->SeedSelectedCheckBox, 
                SIGNAL(stateChanged(int)),
                SLOT(setSeedSelectedFiducials(int)));

  QObject::connect(d->EnableSeedingCheckBox, 
                SIGNAL(stateChanged(int)),
                SLOT(setEnableSeeding(int)));

  QObject::connect(d->MaxNumberSeedsNumericInput, 
                SIGNAL(valueChanged(int)),
                SLOT(setMaxNumberSeeds(int)));
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setTractographyFiducialSeedingNode(vtkMRMLNode *node)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  vtkMRMLTractographyFiducialSeedingNode *paramNode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(node);

  if (paramNode && this->mrmlScene())
  {
    this->connectNodeObservers(paramNode);

    this->updateWidgetfromMRML(paramNode);
  }

  vtkSlicerTractographyFiducialSeedingLogic *seedingLogic = 
        vtkSlicerTractographyFiducialSeedingLogic::SafeDownCast(this->logic());
  if (seedingLogic && this->mrmlScene())
  {
    seedingLogic->SetAndObserveTractographyFiducialSeedingNode(paramNode);
    if (this->TractographyFiducialSeedingNode != paramNode && paramNode)
    {
      seedingLogic->ProcessMRMLEvents (NULL, 
                                       vtkCommand::ModifiedEvent,
                                       paramNode );
    }

  }

  this->TractographyFiducialSeedingNode = paramNode;
}


//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setTransformableNode(vtkMRMLNode *node)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  this->TransformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
  if (this->TransformableNode) 
  {
    d->FiducialNodeSelector->setCurrentNode(this->TransformableNode);
    if (this->TractographyFiducialSeedingNode)
    {
      this->TractographyFiducialSeedingNode->SetInputFiducialRef(this->TransformableNode->GetID() );
      this->connectNodeObservers(this->TractographyFiducialSeedingNode);
    }
  }
}
//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setDiffusionTensorVolumeNode(vtkMRMLNode *node)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  this->DiffusionTensorVolumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(node);
  if (this->DiffusionTensorVolumeNode) 
  {
    d->DTINodeSelector->setCurrentNode(this->DiffusionTensorVolumeNode);
    if (this->TractographyFiducialSeedingNode)
    {
      this->TractographyFiducialSeedingNode->SetInputVolumeRef(this->DiffusionTensorVolumeNode->GetID() );
      this->connectNodeObservers(TractographyFiducialSeedingNode);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setFiberBundleNode(vtkMRMLNode *node)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  this->FiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(node);
  if (this->FiberBundleNode) 
  {
    d->FiberNodeSelector->setCurrentNode(this->FiberBundleNode);
    if (this->TractographyFiducialSeedingNode)
    {
      this->TractographyFiducialSeedingNode->SetOutputFiberRef(this->FiberBundleNode->GetID() );
    }
  }
}



//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::onParameterChanged(vtkObject* node)
{
  Q_UNUSED(node);
  vtkSlicerTractographyFiducialSeedingLogic *seedingLogic = 
        vtkSlicerTractographyFiducialSeedingLogic::SafeDownCast(this->logic());
  if (seedingLogic && this->mrmlScene() && this->TractographyFiducialSeedingNode)
  {
    seedingLogic->ProcessMRMLEvents (NULL, 
                                     vtkCommand::ModifiedEvent,
                                     this->TractographyFiducialSeedingNode );
  }
}

//-----------------------------------------------------------------------------

void qSlicerTractographyFiducialSeedingModuleWidget::connectNodeObservers(vtkMRMLTractographyFiducialSeedingNode* paramNode)
{
  if (paramNode && this->mrmlScene())
  {
    if (paramNode->GetInputFiducialRef() )
    {
      vtkMRMLTransformableNode *transformableNode = vtkMRMLTransformableNode::SafeDownCast(this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()));
      if (transformableNode)
      {
        this->qvtkReconnect(this->TransformableNode, transformableNode,
                      vtkMRMLTransformableNode::TransformModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));
        this->qvtkReconnect(this->TransformableNode, transformableNode,
                      vtkMRMLModelNode::PolyDataModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));
        this->qvtkReconnect(this->TransformableNode, transformableNode,
                      vtkMRMLFiducialListNode::FiducialModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));
        this->TransformableNode = transformableNode;
      }
    }
    if (paramNode->GetInputVolumeRef() )
    {
      vtkMRMLDiffusionTensorVolumeNode *diffusionTensorVolumeNode = vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(this->mrmlScene()->GetNodeByID(paramNode->GetInputVolumeRef()));
      if (diffusionTensorVolumeNode)
      {
        this->qvtkReconnect(this->DiffusionTensorVolumeNode, diffusionTensorVolumeNode,
                      vtkCommand::ModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));
        this->DiffusionTensorVolumeNode = diffusionTensorVolumeNode;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::
  updateWidgetfromMRML(vtkMRMLTractographyFiducialSeedingNode *paramNode)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (paramNode && this->mrmlScene())
    {

    d->IntegrationStepSpinBoxLabel->setValue(paramNode->GetIntegrationStep());
    d->MaxNumberSeedsNumericInput->setValue(paramNode->GetMaxNumberOfSeeds());
    d->MinimumPathSpinBoxLabel->setValue(paramNode->GetMinimumPathLength());
    d->FiducialRegionSpinBoxLabel->setValue(paramNode->GetSeedingRegionSize());
    d->FiducialStepSpinBoxLabel->setValue(paramNode->GetSeedingRegionStep());
    d->SeedSelectedCheckBox->setChecked(paramNode->GetSeedSelectedFiducials()==1);
    d->StoppingCurvatureSpinBoxLabel->setValue(paramNode->GetStoppingCurvature());
    d->StoppingCriteriaComboBox->setCurrentIndex(paramNode->GetStoppingMode());
    d->StoppingValueSpinBoxLabel->setValue(paramNode->GetStoppingValue());
    d->DisplayTracksComboBox->setCurrentIndex(paramNode->GetDisplayMode());

    d->ParameterNodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetID()));
    d->FiberNodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetOutputFiberRef()));
    d->FiducialNodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()));
    d->DTINodeSelector->setCurrentNode(
      this->mrmlScene()->GetNodeByID(paramNode->GetInputVolumeRef()));

    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setStoppingCurvature(double value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetStoppingCurvature(value);
  }
  d->StoppingCurvatureSpinBoxLabel->setValue(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setStoppingValue(double value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetStoppingValue(value);
  }
  d->StoppingValueSpinBoxLabel->setValue(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setIntegrationStep(double value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetIntegrationStep(value);
  }
  d->IntegrationStepSpinBoxLabel->setValue(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setMinimumPath(double value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetMinimumPathLength(value);
  }
  d->MinimumPathSpinBoxLabel->setValue(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setFiducialRegion(double value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetSeedingRegionSize(value);
  }
  d->FiducialRegionSpinBoxLabel->setValue(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setFiducialRegionStep(double value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);
  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetSeedingRegionStep(value);
  }
  d->FiducialStepSpinBoxLabel->setValue(value);
}


//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setStoppingCriteria(int value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetStoppingMode(value);
  }
  d->StoppingCriteriaComboBox->setCurrentIndex(value);
}
//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setTrackDisplayMode(int value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetDisplayMode(value);
  }
  d->DisplayTracksComboBox->setCurrentIndex(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setSeedSelectedFiducials(int value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetSeedSelectedFiducials(value);
  }
  d->SeedSelectedCheckBox->setChecked(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setMaxNumberSeeds(int value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetMaxNumberOfSeeds(value);
  }
  d->MaxNumberSeedsNumericInput->setValue(value);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setEnableSeeding(int value)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);
 if (this->TractographyFiducialSeedingNode)
  {
    this->TractographyFiducialSeedingNode->SetEnableSeeding(value);
  }
 d->EnableSeedingCheckBox->setChecked(value);
}

