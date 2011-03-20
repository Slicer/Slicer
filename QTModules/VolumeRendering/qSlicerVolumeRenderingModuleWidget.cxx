// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModule.h"
#include "vtkMRMLVolumeRenderingParametersNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"

#include <QtGui/QLabel>
#include "qMRMLNodeComboBox.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScalarVolumeNode.h"

//-----------------------------------------------------------------------------
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



}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLParameterNode(vtkMRMLNode* paramNode)
{
  if (paramNode == NULL)
    {
    return;
    }
  vtkMRMLScene* scene = paramNode->GetScene();
  if (scene == NULL)
    {
    return;
    }

  // use vtkMRMLVolumeRenderingScenarioNode (assume singletone)
  // to hold current parameter node
  vtkMRMLVolumeRenderingScenarioNode *snode = vtkMRMLVolumeRenderingScenarioNode::SafeDownCast(
    scene->GetNthNodeByClass(0, "vtkMRMLVolumeRenderingScenarioNode"));

  if (snode == NULL)
    {
    snode = vtkMRMLVolumeRenderingScenarioNode::New();
    snode->SetParametersNodeID(paramNode->GetID());
    scene->AddNode(snode);
    snode->Delete();
    }

  snode->SetParametersNodeID(paramNode->GetID());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumePropertyNode(vtkMRMLNode* volumePropertyNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
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
  if (!volumeRenderingParametersNode)
  {
    volumeRenderingParametersNode = this->createParameterNode();
  }
  volumeRenderingParametersNode->SetAndObserveVolumePropertyNodeID(volumePropertyNode->GetID());
}
  


// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLROINode(vtkMRMLNode* roiNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
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
  if (!volumeRenderingParametersNode)
  {
    volumeRenderingParametersNode = this->createParameterNode();
  }
  volumeRenderingParametersNode->SetAndObserveROINodeID(roiNode->GetID());
}
  
// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  if (volumeNode == NULL)
    {
    return;
    }
  vtkMRMLScene* scene = volumeNode->GetScene();
  if (scene == NULL)
    {
    return;
    }

  vtkMRMLScalarVolumeNode* scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);

  if (scalarVolumeNode)
  {
    vtkMRMLVolumeRenderingParametersNode* volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
      d->ParameterNodeSelector->currentNode());
    if (!volumeRenderingParametersNode)
    {
      volumeRenderingParametersNode = this->createParameterNode();
      this->setMRMLParameterNode(volumeRenderingParametersNode);
    }
    volumeRenderingParametersNode->SetAndObserveVolumeNodeID(scalarVolumeNode->GetID());

    if (volumeRenderingParametersNode->GetVolumePropertyNode() == 0)
    {
      vtkMRMLVolumePropertyNode* vpNode =  this->createVolumeProprtyNode();
      this->setMRMLVolumePropertyNode(vpNode);
    }
  }
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingParametersNode* qSlicerVolumeRenderingModuleWidget::createParameterNode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLScene* scene = this->mrmlScene();

  vtkMRMLVolumeRenderingParametersNode *volumeRenderingParametersNode = vtkMRMLVolumeRenderingParametersNode::New();
  volumeRenderingParametersNode->SetCurrentVolumeMapper(0);
  scene->AddNode(volumeRenderingParametersNode);
  volumeRenderingParametersNode->Delete();
  if (d->ActiveVolumeNodeSelector->currentNode())
  {
    volumeRenderingParametersNode->SetAndObserveVolumeNodeID(d->ActiveVolumeNodeSelector->currentNode()->GetID());
  }
  if (d->ROINodeSelector->currentNode())
  {
    volumeRenderingParametersNode->SetAndObserveROINodeID(d->ROINodeSelector->currentNode()->GetID());
  }
  if (d->VolumePropertyNodeSelector->currentNode())
  {
    volumeRenderingParametersNode->SetAndObserveVolumePropertyNodeID(d->VolumePropertyNodeSelector->currentNode()->GetID());
  }
  d->ParameterNodeSelector->setCurrentNode(volumeRenderingParametersNode);
  return volumeRenderingParametersNode;
}

// --------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* qSlicerVolumeRenderingModuleWidget::createVolumeProprtyNode()
{
  vtkMRMLScene* scene = this->mrmlScene();

  vtkMRMLVolumePropertyNode *volumePropertyNode = vtkMRMLVolumePropertyNode::New();
  scene->AddNode(volumePropertyNode);
  volumePropertyNode->Delete();
  return volumePropertyNode;
}
