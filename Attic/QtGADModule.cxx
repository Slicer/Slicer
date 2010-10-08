
//# ifdef Slicer_USE_PYTHON
//#   include "PythonQt.h"
//#   include "PythonQt_QtAll.h"
//# endif
#include <QApplication>
#include <QTextBrowser>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

#include "QtSlicerNodeSelectorWidget.h"
#include "QtGADModule.h"

#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkGradientAnisotropicDiffusionFilterLogic.h"

#include "vtkEventQtSlotConnect.h"

QtGADModule::QtGADModule()
{
  this->Logic = vtkGradientAnisotropicDiffusionFilterLogic::New();
  this->CreatingParametersNode = 0;
  this->UpdatingMRML = 0;
  this->UpdatingGUI = 0;
}

QtGADModule::~QtGADModule()
{
  this->Logic->Delete();
}
void QtGADModule::BuildGUI()
{

  // create a small Qt GUI
  QVBoxLayout*  vbox = new QVBoxLayout;
  QGroupBox*    box  = new QGroupBox;

  QHBoxLayout*  hbox01 = new QHBoxLayout;
  QLabel *paramLabel = new QLabel("Parameters", this);
  this->GADNodeSelector = new QtSlicerNodeSelectorWidget();
  this->GADNodeSelector->setObjectName("parameter selector");
  this->GADNodeSelector->SetMRMLScene(this->MRMLScene);
  this->GADNodeSelector->SetNodeClass("vtkMRMLGradientAnisotropicDiffusionFilterNode", NULL, NULL, NULL);
  this->GADNodeSelector->SetNoneEnabled(1);
  this->GADNodeSelector->SetNewNodeEnabled(1);
  this->GADNodeSelector->SetShowHidden(1);
  hbox01->addWidget(paramLabel);
  hbox01->addWidget(this->GADNodeSelector);
  vbox->addLayout(hbox01);

  QHBoxLayout*  hbox02 = new QHBoxLayout;
  QLabel *inputLabel = new QLabel("Input Volume", this);
  this->VolumeSelector = new QtSlicerNodeSelectorWidget();
  this->VolumeSelector->setObjectName("volume selector");
  this->VolumeSelector->SetMRMLScene(this->MRMLScene);
  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->VolumeSelector->SetNoneEnabled(0);
  this->VolumeSelector->SetNewNodeEnabled(0);
  hbox02->addWidget(inputLabel);
  hbox02->addWidget(this->VolumeSelector);
  vbox->addLayout(hbox02);

  QHBoxLayout*  hbox03 = new QHBoxLayout;
  QLabel *outputLabel = new QLabel("Output Volume", this);
  this->OutVolumeSelector = new QtSlicerNodeSelectorWidget();
  this->OutVolumeSelector->setObjectName("out volume selector");
  this->OutVolumeSelector->SetMRMLScene(this->MRMLScene);
  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->OutVolumeSelector->SetNoneEnabled(0);
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  hbox03->addWidget(outputLabel);
  hbox03->addWidget(this->OutVolumeSelector);
  vbox->addLayout(hbox03);


  QHBoxLayout*  hbox1 = new QHBoxLayout;
  this->ConductanceSlider = new QtSlider(this);
  this->ConductanceSlider->setValue(1.0);
  this->ConductanceSlider->setMinimum(0.0);
  this->ConductanceSlider->setMaximum(10.0);
  QLabel *conductanceLabel = new QLabel("Conductance", this);
  hbox1->addWidget(conductanceLabel);
  hbox1->addWidget(this->ConductanceSlider);
  vbox->addLayout(hbox1);

  QHBoxLayout*  hbox2 = new QHBoxLayout;
  this->TimeStepSlider = new QtSlider(this);
  this->TimeStepSlider->setValue(0.06);
  this->TimeStepSlider->setMinimum(0.001);
  this->TimeStepSlider->setMaximum(1.0);
  QLabel *timeStepLabel = new QLabel("Time Step", this);
  hbox2->addWidget(timeStepLabel);
  hbox2->addWidget(this->TimeStepSlider);
  vbox->addLayout(hbox2);

  QHBoxLayout*  hbox3 = new QHBoxLayout;
  this->IterationsSlider = new QtSlider(this);
  this->IterationsSlider->setValue(1.0);
  this->IterationsSlider->setMinimum(1.0);
  this->IterationsSlider->setMaximum(30.0);
  QLabel *iterationsLabel = new QLabel("Iterations", this);
  hbox3->addWidget(iterationsLabel);
  hbox3->addWidget(this->IterationsSlider);
  vbox->addLayout(hbox3);


  QPushButton*  button = new QPushButton(box);
  button->setObjectName("apply button");
  button->setText("Apply");
  vbox->addWidget(button);

  box->setLayout(vbox);

  this->connect(this->ConductanceSlider, SIGNAL(valueChanged(double)), this, SLOT(parameterValueChanged (double)));
  this->connect(this->TimeStepSlider, SIGNAL(valueChanged(double)), this, SLOT(parameterValueChanged (double)));
  this->connect(this->IterationsSlider, SIGNAL(valueChanged(double)), this, SLOT(parameterValueChanged (double)));
  this->VolumeSelector->connect(button, SIGNAL(clicked()), this, SLOT(apply ()));
  this->GADNodeSelector->connect(this->GADNodeSelector, SIGNAL(NodeSelected(const QString&)), this, SLOT(parameterSelected ( const QString &)));
  this->VolumeSelector->connect(this->VolumeSelector, SIGNAL(NodeSelected(const QString&)), this, SLOT(inputSelected ( const QString &)));
  this->OutVolumeSelector->connect(this->OutVolumeSelector, SIGNAL(NodeSelected(const QString&)), this, SLOT(outputSelected ( const QString &)));

  box->show();

}

void QtGADModule::parameterValueChanged(double value)
{
  this->updateMRML();
}

void QtGADModule::apply()
{
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (!paramNode)
    {
    paramNode = this->CreateParameterNode();
    }
  this->Logic->SetMRMLScene(this->MRMLScene);
  this->Logic->SetAndObserveGradientAnisotropicDiffusionFilterNode(paramNode);
  this->Logic->Apply();
}

void QtGADModule::parameterSelected(const QString& id)
{
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  // Update GUI
  this->updateGUI();
  // Observe parameter node
  vtkEventQtSlotConnect* Connections = vtkEventQtSlotConnect::New();

  Connections->Connect(paramNode, vtkCommand::ModifiedEvent,
                       this, SLOT(updateGUI()));
  this->updateMRML();

}

void QtGADModule::inputSelected(const QString& id)
{
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (!paramNode)
    {
    paramNode = this->CreateParameterNode();
    }
  if (paramNode)
    {
    paramNode->SetInputVolumeRef(id.toStdString().c_str());
    }
}
void QtGADModule::outputSelected(const QString& id)
{
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (!paramNode)
    {
    paramNode = this->CreateParameterNode();
    }
  if (paramNode)
    {
    paramNode->SetOutputVolumeRef (id.toStdString().c_str());
    }
}

vtkMRMLGradientAnisotropicDiffusionFilterNode* QtGADModule::CreateParameterNode()
{
  if (this->CreatingParametersNode)
    {
    return NULL;
    }

  this->CreatingParametersNode = 1;

  this->GADNodeSelector->SetSelectedNew("vtkMRMLGradientAnisotropicDiffusionFilterNode");
  this->GADNodeSelector->ProcessNewNodeCommand("vtkMRMLGradientAnisotropicDiffusionFilterNode", "GADParameters");
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());

  this->updateMRML();

  this->CreatingParametersNode = 0;

  return paramNode;
}

void QtGADModule::updateGUI()
{
  if (this->UpdatingMRML || this->UpdatingGUI)
    {
    return;
    }
  this->UpdatingGUI = 1;

  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (paramNode)
    {
    this->VolumeSelector->SetSelected(this->MRMLScene->GetNodeByID(paramNode->GetInputVolumeRef()));
    this->OutVolumeSelector->SetSelected(this->MRMLScene->GetNodeByID(paramNode->GetOutputVolumeRef()));
    this->ConductanceSlider->setValue(paramNode->GetConductance());
    this->TimeStepSlider->setValue(paramNode->GetTimeStep());
    this->IterationsSlider->setValue(paramNode->GetNumberOfIterations());
    }

  this->UpdatingGUI = 0;

}

void QtGADModule::updateMRML()
{
  if (this->UpdatingMRML || this->UpdatingGUI)
    {
    return;
    }
  this->UpdatingMRML = 1;


  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (paramNode)
    {
    if (this->VolumeSelector->GetSelected())
      {
      paramNode->SetInputVolumeRef (this->VolumeSelector->GetSelected()->GetID());
      }
    if (this->OutVolumeSelector->GetSelected())
      {
      paramNode->SetOutputVolumeRef (this->OutVolumeSelector->GetSelected()->GetID());
      }
    paramNode->SetConductance(this->ConductanceSlider->value());
    paramNode->SetTimeStep(this->TimeStepSlider->value());
    paramNode->SetNumberOfIterations(this->IterationsSlider->value());
    }

  this->UpdatingMRML = 0;
}


