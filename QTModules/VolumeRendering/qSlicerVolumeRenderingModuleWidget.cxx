// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModule.h"
#include "vtkMRMLVolumeRenderingParametersNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"
#include "vtkMRMLViewNode.h"

#include <QtGui/QLabel>
#include "qMRMLNodeComboBox.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScalarVolumeNode.h"

#include "vtkSlicerVolumeRenderingLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingModuleWidgetPrivate: public Ui_qSlicerVolumeRenderingModule
{
public:
  qSlicerVolumeRenderingModuleWidgetPrivate();

  QLabel *ActiveVolumeLabel;
  qMRMLNodeComboBox *ActiveVolumeNodeSelector;

  QLabel *ParameterLabel;
  qMRMLNodeComboBox *ParameterNodeSelector;

  QLabel *ROILabel;
  qMRMLNodeComboBox *ROINodeSelector;

  QLabel *VolumePropertyLabel;
  qMRMLNodeComboBox *VolumePropertyNodeSelector;

  QLabel *ViewLabel;
  qMRMLNodeComboBox *ViewNodeSelector;

};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidgetPrivate::qSlicerVolumeRenderingModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::qSlicerVolumeRenderingModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerVolumeRenderingModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::~qSlicerVolumeRenderingModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setup()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->setupUi(this);
  //this->Superclass::setup();

  d->ActiveVolumeLabel = new QLabel(this);
  d->ActiveVolumeLabel->setObjectName(QString::fromUtf8("ActiveVolumeLabel"));
  d->ActiveVolumeLabel->setText(QString::fromUtf8("Volume"));
  d->gridLayout->addWidget(d->ActiveVolumeLabel, 1, 0, 1, 1);

  d->ActiveVolumeNodeSelector = new qMRMLNodeComboBox(this);
  d->ActiveVolumeNodeSelector->setObjectName(QString::fromUtf8("ActiveVolumeNodeSelector"));
  d->ActiveVolumeNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLScalarVolumeNode"));
  d->ActiveVolumeNodeSelector->setAddEnabled(false);
  d->ActiveVolumeNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->ActiveVolumeNodeSelector, 1, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ActiveVolumeNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));

  d->ParameterLabel = new QLabel(this);
  d->ParameterLabel->setObjectName(QString::fromUtf8("ParameterLabel"));
  d->ParameterLabel->setText(QString::fromUtf8("Parameter"));
  d->gridLayout->addWidget(d->ParameterLabel, 0, 0, 1, 1);

  d->ParameterNodeSelector = new qMRMLNodeComboBox(this);
  d->ParameterNodeSelector->setObjectName(QString::fromUtf8("ParameterNodeSelector"));
  d->ParameterNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLVolumeRenderingParametersNode"));
  d->ParameterNodeSelector->setAddEnabled(false);
  d->ParameterNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->ParameterNodeSelector, 0, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ParameterNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ParameterNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLParameterNode(vtkMRMLNode*)));

  d->ROILabel = new QLabel(this);
  d->ROILabel->setObjectName(QString::fromUtf8("ROILabel"));
  d->ROILabel->setText(QString::fromUtf8("ROI"));
  d->gridLayout->addWidget(d->ROILabel, 2, 0, 1, 1);

  d->ROINodeSelector = new qMRMLNodeComboBox(this);
  d->ROINodeSelector->setObjectName(QString::fromUtf8("ROINodeSelector"));
  d->ROINodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLROINode"));
  d->ROINodeSelector->setAddEnabled(false);
  d->ROINodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->ROINodeSelector, 2, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ROINodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ROINodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLROINode(vtkMRMLNode*)));

  d->VolumePropertyLabel = new QLabel(this);
  d->VolumePropertyLabel->setObjectName(QString::fromUtf8("VolumePropertyLabel"));
  d->VolumePropertyLabel->setText(QString::fromUtf8("VolumeProperty"));
  d->gridLayout->addWidget(d->VolumePropertyLabel, 3, 0, 1, 1);

  d->VolumePropertyNodeSelector = new qMRMLNodeComboBox(this);
  d->VolumePropertyNodeSelector->setObjectName(QString::fromUtf8("VolumePropertyNodeSelector"));
  d->VolumePropertyNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLVolumePropertyNode"));
  d->VolumePropertyNodeSelector->setAddEnabled(false);
  d->VolumePropertyNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->VolumePropertyNodeSelector, 3, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->VolumePropertyNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->VolumePropertyNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLVolumePropertyNode(vtkMRMLNode*)));

  d->ViewLabel = new QLabel(this);
  d->ViewLabel->setObjectName(QString::fromUtf8("ViewLabel"));
  d->ViewLabel->setText(QString::fromUtf8("View"));
  d->gridLayout->addWidget(d->ViewLabel, 4, 0, 1, 1);

  d->ViewNodeSelector = new qMRMLNodeComboBox(this);
  d->ViewNodeSelector->setObjectName(QString::fromUtf8("ViewNodeSelector"));
  d->ViewNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLViewNode"));
  d->ViewNodeSelector->setAddEnabled(false);
  d->ViewNodeSelector->setRenameEnabled(false);

  d->gridLayout->addWidget(d->ViewNodeSelector, 4, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ViewNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLViewNode(vtkMRMLNode*)));



}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLViewNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLViewNode *viewNode = vtkMRMLViewNode::SafeDownCast(node);

  d->ViewNodeSelector->setCurrentNode(viewNode);

  if (viewNode == NULL)
  {
    return;
  }

  vtkMRMLVolumeRenderingParametersNode* volumeRenderingParametersNode = NULL;
  if (viewNode->GetVolumeRenderingParameterNodeID() && viewNode->GetScene())
  {
    volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
         viewNode->GetScene()->GetNodeByID(viewNode->GetVolumeRenderingParameterNodeID()));
  }
  this->setMRMLParameterNode(volumeRenderingParametersNode);
}

 // --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLVolumeRenderingParametersNode* paramNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(node);

  d->ParameterNodeSelector->setCurrentNode(paramNode);
  if (paramNode == NULL || paramNode->GetScene() == NULL)
    {
    return;
    }

  this->setMRMLVolumeNode(paramNode->GetVolumeNode());
  this->setMRMLVolumePropertyNode(paramNode->GetVolumePropertyNode());
  this->setMRMLROINode(paramNode->GetROINode());

  vtkMRMLViewNode *viewNode = vtkMRMLViewNode::SafeDownCast(d->ViewNodeSelector->currentNode());
  if (viewNode)
  {
    viewNode->SetVolumeRenderingParameterNodeID(paramNode->GetID());
  }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLScalarVolumeNode* scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  d->ActiveVolumeNodeSelector->setCurrentNode(scalarVolumeNode);

  if (scalarVolumeNode == NULL)
    {
    return;
    }
  vtkMRMLScene* scene = scalarVolumeNode->GetScene();
  if (scene == NULL)
    {
    return;
    }

  vtkMRMLVolumeRenderingParametersNode* volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
    d->ParameterNodeSelector->currentNode());
  if (!volumeRenderingParametersNode)
  {
    volumeRenderingParametersNode = this->createParameterNode();
    volumeRenderingParametersNode->SetAndObserveVolumeNodeID(scalarVolumeNode->GetID());
  }

}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumePropertyNode(vtkMRMLNode* volumePropertyNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  
  d->VolumePropertyNodeSelector->setCurrentNode(volumePropertyNode);
  if (volumePropertyNode == NULL)
    {
    return;
    }
  vtkMRMLScene* scene = volumePropertyNode->GetScene();
  if (scene == NULL)
    {
    return;
    }

  vtkMRMLVolumeRenderingParametersNode* volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
                d->ParameterNodeSelector->currentNode());
  if (volumeRenderingParametersNode)
  {
    volumeRenderingParametersNode->SetAndObserveVolumePropertyNodeID(volumePropertyNode->GetID());
  }
}
  


// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLROINode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(node);
  d->ROINodeSelector->setCurrentNode(roiNode);
  if (roiNode == NULL)
    {
    return;
    }
  vtkMRMLScene* scene = roiNode->GetScene();
  if (scene == NULL)
    {
    return;
    }

  vtkMRMLVolumeRenderingParametersNode* volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
    d->ParameterNodeSelector->currentNode());

  if (volumeRenderingParametersNode)
  {
    volumeRenderingParametersNode->SetAndObserveROINodeID(roiNode->GetID());
  }
}
  

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingParametersNode* qSlicerVolumeRenderingModuleWidget::createParameterNode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkSlicerVolumeRenderingLogic *logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  vtkMRMLVolumeRenderingParametersNode *volumeRenderingParametersNode = logic->CreateParametersNode();

  vtkMRMLVolumePropertyNode *propNode = vtkMRMLVolumePropertyNode::SafeDownCast(d->VolumePropertyNodeSelector->currentNode());

  vtkMRMLROINode            *roiNode = vtkMRMLROINode::SafeDownCast(d->ROINodeSelector->currentNode());

  logic->UpdateParametersNodeFromVolumeNode(volumeRenderingParametersNode,
                                            vtkMRMLVolumeNode::SafeDownCast(d->ActiveVolumeNodeSelector->currentNode()),
                                            &propNode, &roiNode);

  setMRMLParameterNode(volumeRenderingParametersNode);
  //d->ParameterNodeSelector->setCurrentNode(volumeRenderingParametersNode);
  //d->VolumePropertyNodeSelector->setCurrentNode(propNode);
  //d->ROINodeSelector->setCurrentNode(roiNode);
  return volumeRenderingParametersNode;
}
