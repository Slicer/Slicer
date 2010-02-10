#include "qSlicerTractographyFiducialSeedingModuleWidget.h"
#include "ui_qSlicerTractographyFiducialSeedingModule.h"

// Qt includes
#include <QDebug>

// Logic includes
#include "vtkSlicerTractographyFiducialSeedingLogic.h"
#include "vtkMRMLTractographyFiducialSeedingNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiducialListNode.h"

//-----------------------------------------------------------------------------
class qSlicerTractographyFiducialSeedingModuleWidgetPrivate: 
  public qCTKPrivate<qSlicerTractographyFiducialSeedingModuleWidget>,
  public Ui_qSlicerTractographyFiducialSeedingModule
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerTractographyFiducialSeedingModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setup()
{
  this->processParameterChange = true;
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);
  d->setupUi(this);

  this->TractographyFiducialSeedingNode = NULL;
  this->FiberBundleNode                 = NULL;
  this->TransformableNode               = NULL;
  this->DiffusionTensorVolumeNode       = NULL;

  QObject::connect(d->ParameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onParameterNodeChanged(vtkMRMLNode*)));
  QObject::connect(d->DTINodeSelector, SIGNAL(currentNodeChanged(bool)), this, SLOT(onSelectionNodeChanged(bool)));
  QObject::connect(d->FiducialNodeSelector, SIGNAL(currentNodeChanged(bool)), this, SLOT(onSelectionNodeChanged(bool)));
  QObject::connect(d->FiberNodeSelector, SIGNAL(currentNodeChanged(bool)), this, SLOT(onSelectionNodeChanged(bool)));

  //QObject::connect(d->StoppingCurvatureSpinBoxLabel, SIGNAL(sliderMoved(double)),
  //              SLOT(onParameterChanged(double)));

  QObject::connect(d->StoppingCurvatureSpinBoxLabel, SIGNAL(valueChanged(double)),
                SLOT(onParameterChanged(double)));
  QObject::connect(d->StoppingCriteriaComboBox, SIGNAL(currentIndexChanged(int)),
                SLOT(onParameterChanged(int)));
  QObject::connect(d->StoppingValueSpinBoxLabel, SIGNAL(valueChanged(double)),
                SLOT(onParameterChanged(double)));
  QObject::connect(d->StoppingCurvatureSpinBoxLabel, SIGNAL(valueChanged(double)),
                SLOT(onParameterChanged(double)));
  QObject::connect(d->IntegrationStepSpinBoxLabel, SIGNAL(valueChanged(double)),
                SLOT(onParameterChanged(double)));
  QObject::connect(d->MinimumPathSpinBoxLabel, SIGNAL(valueChanged(double)),
                SLOT(onParameterChanged(double)));
  QObject::connect(d->FiducialRegionSpinBoxLabel, SIGNAL(valueChanged(double)),
                SLOT(onParameterChanged(double)));
  QObject::connect(d->FiducialStepSpinBoxLabel, SIGNAL(valueChanged(double)),
                SLOT(onParameterChanged(double)));

  QObject::connect(d->DisplayTracksComboBox, SIGNAL(currentIndexChanged(int)),
              SLOT(onParameterChanged(int)));

  QObject::connect(d->SeedSelectedCheckBox, SIGNAL(stateChanged(int)),
              SLOT(onParameterChanged(int)));
  QObject::connect(d->EnableSeedingCheckBox, SIGNAL(stateChanged(int)),
              SLOT(onParameterChanged(int)));
  QObject::connect(d->MaxNumberSeedsNumericInput, SIGNAL(valueChanged(int)),
              SLOT(onParameterChanged(int)));
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::onParameterChanged(double value)
{
  if (!this->processParameterChange)
  {
    return;
  }

  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (!d->SeedSelectedCheckBox->isChecked()) 
    {
    return;
    }

  // run seeding here
  
  vtkSlicerTractographyFiducialSeedingLogic *seedingLogic = 
    vtkSlicerTractographyFiducialSeedingLogic::SafeDownCast(this->logic());

  if (seedingLogic)
  {
    qDebug() << "parameter changed: " << value ;

    vtkMRMLDiffusionTensorVolumeNode *volumeNode = 
      vtkMRMLDiffusionTensorVolumeNode::SafeDownCast( d->DTINodeSelector->currentNode() );
    vtkMRMLTransformableNode *fiducialListNode = 
      vtkMRMLTransformableNode::SafeDownCast( d->FiducialNodeSelector->currentNode() );
    vtkMRMLFiberBundleNode *fiberNode = 
      vtkMRMLFiberBundleNode::SafeDownCast( d->FiberNodeSelector->currentNode() );

    if(volumeNode == NULL || fiducialListNode == NULL || fiberNode == NULL) 
    {
      return;
    }

    std::string stopingMode = d->StoppingCriteriaComboBox->currentText().toStdString();

    this->processParameterChange = false;

    seedingLogic->CreateTracts(volumeNode, fiducialListNode, fiberNode,
                               stopingMode.c_str(),
                               d->StoppingValueSpinBoxLabel->value(),
                               d->StoppingCurvatureSpinBoxLabel->value(),
                               d->IntegrationStepSpinBoxLabel->value(),
                               d->MinimumPathSpinBoxLabel->value(),
                               d->FiducialRegionSpinBoxLabel->value(),
                               d->FiducialStepSpinBoxLabel->value(),
                               d->MaxNumberSeedsNumericInput->value(),
                               (d->SeedSelectedCheckBox->isChecked() ? 1:0),
                               d->DisplayTracksComboBox->currentIndex()
                               ); 

    this->processParameterChange = true;
  }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::onParameterChanged( int value)
{
  Q_UNUSED(value);
  this->onParameterChanged((double)0);
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::onParameterNodeChanged(vtkMRMLNode *node)
{
  if (!this->processParameterChange)
  {
    return;
  }
 
  this->processParameterChange = false;

  //QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  std::cout << "param node changed(" << node << "): TODO update sliders with new values\n";
  vtkMRMLTractographyFiducialSeedingNode *paramNode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(node);
  if (paramNode && this->mrmlScene())
  {
    this->qvtkReconnect(this->TractographyFiducialSeedingNode, paramNode,
                        vtkCommand::ModifiedEvent, this, SLOT(onParameterNodeModified(vtkObject*)));

    this->connectNodeObservers(paramNode);

    this->updateWidgetfromMRML(paramNode);

  }
  this->onParameterChanged(0.0);

  this->processParameterChange = true;

}

//-----------------------------------------------------------------------------

void qSlicerTractographyFiducialSeedingModuleWidget::onParameterNodeModified(vtkObject* node)
{
  if (!this->processParameterChange)
  {
    return;
  }

  this->processParameterChange = false;

  //QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  vtkMRMLTractographyFiducialSeedingNode *paramNode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(node);
  if (paramNode && this->mrmlScene())
  {
    this->connectNodeObservers(paramNode);

    this->updateWidgetfromMRML(paramNode);
  }
  this->onParameterChanged(0.0);

  this->processParameterChange = true;
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::updateWidgetfromMRML(vtkMRMLTractographyFiducialSeedingNode *paramNode)
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  if (paramNode && this->mrmlScene())
    {
    this->processParameterChange = false;

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

    d->FiberNodeSelector->setCurrentNode( this->mrmlScene()->GetNodeByID(paramNode->GetOutputFiberRef()) );
    d->FiducialNodeSelector->setCurrentNode( this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()) );
    d->DTINodeSelector->setCurrentNode( this->mrmlScene()->GetNodeByID(paramNode->GetInputVolumeRef()) );

    this->processParameterChange = true;
    }
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::onSelectionNodeChanged(bool value)
{
  if (!this->processParameterChange)
  {
    return;
  }

  //this->processParameterChange = false;

  Q_UNUSED(value);
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);
  vtkMRMLTractographyFiducialSeedingNode *paramNode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(d->ParameterNodeSelector->currentNode());
  if (paramNode && this->mrmlScene())
  {
    if (d->FiberNodeSelector->currentNode())
    {
      paramNode->SetOutputFiberRef(d->FiberNodeSelector->currentNode()->GetID() );
    }
    if (d->FiducialNodeSelector->currentNode())
    {
      paramNode->SetInputFiducialRef(d->FiducialNodeSelector->currentNode()->GetID() );
    }
    if (d->DTINodeSelector->currentNode())
    {
      paramNode->SetInputVolumeRef(d->DTINodeSelector->currentNode()->GetID() );
    }
    this->connectNodeObservers(paramNode);
  }
  this->onParameterChanged(0.0);

  //this->processParameterChange = true;
}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::onParameterChanged(vtkObject* node)
{
  Q_UNUSED(node);
  this->onParameterChanged(0.0);
}

//-----------------------------------------------------------------------------

void qSlicerTractographyFiducialSeedingModuleWidget::connectNodeObservers(vtkMRMLTractographyFiducialSeedingNode* paramNode)
{
  if (paramNode && this->mrmlScene())
  {
    if (paramNode->GetOutputFiberRef() && this->mrmlScene()->GetNodeByID(paramNode->GetOutputFiberRef()) )
    {
      this->qvtkReconnect(this->FiberBundleNode, this->mrmlScene()->GetNodeByID(paramNode->GetOutputFiberRef()),
                    vtkCommand::ModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));

    }
    if (paramNode->GetInputFiducialRef() && this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()) )
    {
      this->qvtkReconnect(this->TransformableNode, this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()),
                    vtkMRMLTransformableNode::TransformModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));
      this->qvtkReconnect(this->TransformableNode, this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()),
                    vtkMRMLModelNode::PolyDataModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));
      this->qvtkReconnect(this->TransformableNode, this->mrmlScene()->GetNodeByID(paramNode->GetInputFiducialRef()),
                    vtkMRMLFiducialListNode::FiducialModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));

    }
    if (paramNode->GetInputVolumeRef() && this->mrmlScene()->GetNodeByID(paramNode->GetInputVolumeRef()) )
    {
      this->qvtkReconnect(this->DiffusionTensorVolumeNode, this->mrmlScene()->GetNodeByID(paramNode->GetInputVolumeRef()),
                    vtkCommand::ModifiedEvent, this, SLOT(onParameterChanged(vtkObject*)));

    }
  }
}
