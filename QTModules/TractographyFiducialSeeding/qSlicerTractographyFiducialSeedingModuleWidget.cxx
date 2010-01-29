#include "qSlicerTractographyFiducialSeedingModuleWidget.h"
#include "ui_qSlicerTractographyFiducialSeedingModule.h"

#include "vtkSlicerTractographyFiducialSeedingLogic.h"
#include "vtkMRMLTractographyFiducialSeedingNode.h"

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

  // run seeding here
  qDebug() << "parameter changed: " << value ;
  // std::cout << "param changed(" << value << "): TODO call logic to compute fibers\n";
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
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);

  std::cout << "param node changed(" << node << "): TODO update sliders with new values\n";
  vtkMRMLTractographyFiducialSeedingNode *paramNode = vtkMRMLTractographyFiducialSeedingNode::SafeDownCast(node);
  if (paramNode)
    {
    this->processParameterChange = false;

    d->IntegrationStepSpinBoxLabel->setValue(paramNode->GetIntegrationStep());
    d->MaxNumberSeedsNumericInput->setValue(paramNode->GetMaxNumberOfSeeds());
    d->MinimumPathSpinBoxLabel->setValue(paramNode->GetMinimumPathLength());
    d->FiducialRegionSpinBoxLabel->setValue(paramNode->GetSeedingRegionSize());
    d->FiducialStepSpinBoxLabel->setValue(paramNode->GetSeedingRegionStep());
    d->SeedSelectedCheckBox->setChecked(paramNode->GetSeedSelectedFiducials()==1);
    d->StoppingCurvatureSpinBoxLabel->setValue(paramNode->GetStoppingCurvature());
    //d->StoppingCriteriaComboBox->setValue(paramNode->GetStoppingMode());
    d->StoppingValueSpinBoxLabel->setValue(paramNode->GetStoppingValue());

    //d->FiberNodeSelector->setCurrentNode(getMRMLScene()->GetNodeByID((paramNode->GetOutputFiberRef()0);
   // ->setValue(paramNode->GetInputFiducialRef());
   // ->setValue(paramNode->GetInputVolumeRef());

    this->processParameterChange = true;
    }
  this->onParameterChanged(0.0);

}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::onSelectionNodeChanged(bool value)
{
  Q_UNUSED(value);
  this->onParameterChanged(0);
}
