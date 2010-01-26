#include "qSlicerTractographyFiducialSeedingModuleWidget.h"
#include "ui_qSlicerTractographyFiducialSeedingModule.h"

//-----------------------------------------------------------------------------
class qSlicerTractographyFiducialSeedingModuleWidgetPrivate: 
  public qCTKPrivate<qSlicerTractographyFiducialSeedingModuleWidget>, public Ui_qSlicerTractographyFiducialSeedingModule
{
};

//-----------------------------------------------------------------------------
qSlicerTractographyFiducialSeedingModuleWidget::qSlicerTractographyFiducialSeedingModuleWidget( QWidget* parent)
  :qSlicerAbstractModuleWidget(parent)
{
  QCTK_INIT_PRIVATE(qSlicerTractographyFiducialSeedingModuleWidget);

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
  QObject::connect(d->MaxNumberSeedsNumericInput, SIGNAL(valueEdited(double)),
              SLOT(onParameterChanged(double)));

}

//-----------------------------------------------------------------------------
void qSlicerTractographyFiducialSeedingModuleWidget::setup()
{
  QCTK_D(qSlicerTractographyFiducialSeedingModuleWidget);
  //d->setupUi(this);
}

void qSlicerTractographyFiducialSeedingModuleWidget::onParameterChanged(double value)
{
  // run seeding here
  std::cout << "param changed: TODO call logic to compute fibers\n";
}

void qSlicerTractographyFiducialSeedingModuleWidget::onParameterChanged( int value)
{
  this->onParameterChanged((double)0);
}

void qSlicerTractographyFiducialSeedingModuleWidget::onParameterNodeChanged(vtkMRMLNode *node)
{

  std::cout << "param node changed: TODO update sliders with new values\n";
  this->onParameterChanged(0.0);

}

void qSlicerTractographyFiducialSeedingModuleWidget::onSelectionNodeChanged(bool value)
{
  this->onParameterChanged(0);
}
