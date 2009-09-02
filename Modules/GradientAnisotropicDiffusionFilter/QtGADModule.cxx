
//# ifdef Slicer3_USE_PYTHON
//#   include "PythonQt.h"
//#   include "PythonQt_QtAll.h"
//# endif
#include <QApplication>
#include <QTextBrowser>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>


#include "QtSlicerNodeSelectorWidget.h"
#include "QtGADModule.h"

#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkGradientAnisotropicDiffusionFilterLogic.h"

#include "vtkEventQtSlotConnect.h"

QtGADModule::QtGADModule()
{
  this->Logic = vtkGradientAnisotropicDiffusionFilterLogic::New();
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

  this->GADNodeSelector = new QtSlicerNodeSelectorWidget();
  this->GADNodeSelector->setObjectName("parameter selector");
  this->GADNodeSelector->SetMRMLScene(this->MRMLScene);
  this->GADNodeSelector->SetNodeClass("vtkMRMLGradientAnisotropicDiffusionFilterNode", NULL, NULL, NULL);
  this->GADNodeSelector->SetNoneEnabled(1);
  this->GADNodeSelector->SetNewNodeEnabled(1);
  this->GADNodeSelector->SetShowHidden(1);
  vbox->addWidget(this->GADNodeSelector);

  this->VolumeSelector = new QtSlicerNodeSelectorWidget();
  this->VolumeSelector->setObjectName("volume selector");
  this->VolumeSelector->SetMRMLScene(this->MRMLScene);
  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->VolumeSelector->SetNoneEnabled(0);
  this->VolumeSelector->SetNewNodeEnabled(0);
  vbox->addWidget(this->VolumeSelector);

  this->OutVolumeSelector = new QtSlicerNodeSelectorWidget();
  this->OutVolumeSelector->setObjectName("out volume selector");
  this->OutVolumeSelector->SetMRMLScene(this->MRMLScene);
  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->OutVolumeSelector->SetNoneEnabled(0);
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  vbox->addWidget(this->OutVolumeSelector);

  QPushButton*  button = new QPushButton(box);
  button->setObjectName("apply button");
  button->setText("Apply");
  vbox->addWidget(button);

  box->setLayout(vbox);

  this->VolumeSelector->connect(button, SIGNAL(clicked()), this, SLOT(apply ()));
  this->GADNodeSelector->connect(this->GADNodeSelector, SIGNAL(NodeSelected(const QString&)), this, SLOT(parameterSelected ( const QString &)));
  this->VolumeSelector->connect(this->VolumeSelector, SIGNAL(NodeSelected(const QString&)), this, SLOT(inputSelected ( const QString &)));
  this->OutVolumeSelector->connect(this->OutVolumeSelector, SIGNAL(NodeSelected(const QString&)), this, SLOT(outputSelected ( const QString &)));

  box->show();

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

}

void QtGADModule::inputSelected(const QString& id)
{
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (!paramNode)
    {
    paramNode = this->CreateParameterNode();
    }
  paramNode->SetInputVolumeRef(id.toStdString().c_str());

}
void QtGADModule::outputSelected(const QString& id)
{
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (!paramNode)
    {
    paramNode = this->CreateParameterNode();
    }
  paramNode->SetOutputVolumeRef (id.toStdString().c_str());
}

vtkMRMLGradientAnisotropicDiffusionFilterNode* QtGADModule::CreateParameterNode()
{
  this->GADNodeSelector->SetSelectedNew("vtkMRMLGradientAnisotropicDiffusionFilterNode");
  this->GADNodeSelector->ProcessNewNodeCommand("vtkMRMLGradientAnisotropicDiffusionFilterNode", "GADParameters");
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (this->VolumeSelector->GetSelected())
    {
    paramNode->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }
  if (this->OutVolumeSelector->GetSelected())
    {
    paramNode->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }
  return paramNode;
}

void QtGADModule::updateGUI()
{
  vtkMRMLGradientAnisotropicDiffusionFilterNode *paramNode = vtkMRMLGradientAnisotropicDiffusionFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
  if (paramNode)
    {
    this->VolumeSelector->SetSelected(this->MRMLScene->GetNodeByID(paramNode->GetInputVolumeRef()));
    this->OutVolumeSelector->SetSelected(this->MRMLScene->GetNodeByID(paramNode->GetOutputVolumeRef()));
    }
}
