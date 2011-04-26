// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModule.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
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
  qMRMLNodeComboBox *DisplayNodeSelector;

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
  , settingVolumeNode(false)
  , settingDisplayNode(false)
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
  d->ActiveVolumeNodeSelector->setNoneEnabled(true);
  d->ActiveVolumeNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->ActiveVolumeNodeSelector, 1, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ActiveVolumeNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));

  d->ParameterLabel = new QLabel(this);
  d->ParameterLabel->setObjectName(QString::fromUtf8("DisplayLabel"));
  d->ParameterLabel->setText(QString::fromUtf8("Display Node"));
  d->gridLayout->addWidget(d->ParameterLabel, 0, 0, 1, 1);


  d->DisplayNodeSelector = new qMRMLNodeComboBox(this);
  d->DisplayNodeSelector->setObjectName(QString::fromUtf8("DisplayNodeSelector"));
  d->DisplayNodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLVolumeRenderingDisplayNode"));
  d->DisplayNodeSelector->setAddEnabled(true);
  d->DisplayNodeSelector->setNoneEnabled(false);
  d->DisplayNodeSelector->setRenameEnabled(true);

  d->gridLayout->addWidget(d->DisplayNodeSelector, 0, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->DisplayNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->DisplayNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLDisplayNode(vtkMRMLNode*)));

  d->ROILabel = new QLabel(this);
  d->ROILabel->setObjectName(QString::fromUtf8("ROILabel"));
  d->ROILabel->setText(QString::fromUtf8("ROI"));
  d->gridLayout->addWidget(d->ROILabel, 2, 0, 1, 1);

  d->ROINodeSelector = new qMRMLNodeComboBox(this);
  d->ROINodeSelector->setObjectName(QString::fromUtf8("ROINodeSelector"));
  d->ROINodeSelector->setNodeTypes(QStringList() << QString::fromUtf8("vtkMRMLROINode"));
  d->ROINodeSelector->setAddEnabled(true);
  d->ROINodeSelector->setNoneEnabled(true);
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
  d->VolumePropertyNodeSelector->setAddEnabled(true);
  d->VolumePropertyNodeSelector->setNoneEnabled(true);
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
  d->ViewNodeSelector->setNoneEnabled(true);
  d->ViewNodeSelector->setRenameEnabled(false);

  d->gridLayout->addWidget(d->ViewNodeSelector, 4, 1, 1, 1);

  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->ViewNodeSelector, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->ViewNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setMRMLViewNode(vtkMRMLNode*)));



}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLViewNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLViewNode *viewNodeOld = vtkMRMLViewNode::SafeDownCast(
                                  d->ViewNodeSelector->currentNode());

  vtkMRMLViewNode *viewNode = vtkMRMLViewNode::SafeDownCast(node);

  d->ViewNodeSelector->setCurrentNode(viewNode);


  // set view in the currently selected display node
  vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
              d->DisplayNodeSelector->currentNode());
  if (displayNode)
  {
    if (viewNodeOld != NULL && viewNodeOld != viewNode)
    {
      displayNode->RemoveAllViewNodeIDs();
    }
    if (viewNode != NULL)
    {
      displayNode->AddViewNodeID(viewNode->GetID());
    }
  }

}

 // --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLDisplayNode(vtkMRMLNode* node)
{
  if (settingDisplayNode)
  {
    return;
  }
  settingDisplayNode = true;

  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);

  vtkMRMLVolumeRenderingDisplayNode* displayNodeOld = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(d->DisplayNodeSelector->currentNode());
  
  if (displayNodeOld && displayNodeOld != displayNode)
    {
    displayNodeOld->RemoveAllViewNodeIDs();
    }

  d->DisplayNodeSelector->setCurrentNode(displayNode);
  if (displayNode == NULL || displayNode->GetScene() == NULL)
    {
    settingDisplayNode = false;
    return;
    }

  vtkSlicerVolumeRenderingLogic *logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  // set display node from current GUI state
  this->setMRMLVolumePropertyNode(displayNode->GetVolumePropertyNode());
  this->setMRMLROINode(displayNode->GetROINode());

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
                d->ActiveVolumeNodeSelector->currentNode());

  // update view node refrences
  vtkMRMLViewNode *viewNode = vtkMRMLViewNode::SafeDownCast(d->ViewNodeSelector->currentNode());
  vtkMRMLVolumeRenderingDisplayNode* dnode = logic->GetVolumeRenderingDisplayNodeForViewNode(volumeNode, 
                                                                                   viewNode);
  if (viewNode && volumeNode && dnode != displayNode)
  {
    logic->RemoveViewFromVolumeDisplayNodes(volumeNode, viewNode);
  } 

  if (displayNode && viewNode)
  {
  displayNode->AddViewNodeID(viewNode->GetID());
  }
  // if display node is not referenced by current volume, add the refrence
  if (volumeNode)
  {
    vtkMRMLVolumeRenderingDisplayNode* dnode = logic->GetVolumeRenderingDisplayNodeByID(volumeNode,
                                                                    displayNode->GetID());
    if (dnode == NULL)
    {
      volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }
  }
  
  settingDisplayNode = false;

}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  if (settingVolumeNode)
  {
    return;
  }
  settingVolumeNode = true;
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  d->ActiveVolumeNodeSelector->setCurrentNode(volumeNode);

  if (volumeNode == NULL)
    {
    settingVolumeNode = false;
    return;
    }
  vtkMRMLScene* scene = volumeNode->GetScene();
  if (scene == NULL)
    {
    settingVolumeNode = false;
    return;
    }

  vtkSlicerVolumeRenderingLogic *logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  // see if the volume has any display node for a current viewer
  vtkMRMLViewNode *viewNode = vtkMRMLViewNode::SafeDownCast(d->ViewNodeSelector->currentNode());
  vtkMRMLVolumeRenderingDisplayNode *dnode  = logic->GetVolumeRenderingDisplayNodeForViewNode(volumeNode, viewNode);
  if (!dnode)
  {
    dnode = logic->GetFirstVolumeRenderingDisplayNode(volumeNode);
  }
  if (!dnode)
  {
    dnode = this->createVolumeRenderingDisplayNode();
    volumeNode->SetAndObserveDisplayNodeID(dnode->GetID());
  }

  //d->DisplayNodeSelector->setCurrentNode(dnode);
  this->setMRMLDisplayNode(dnode);

  settingVolumeNode = false;

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

  vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
                d->DisplayNodeSelector->currentNode());
  if (displayNode)
  {
    displayNode->SetAndObserveVolumePropertyNodeID(volumePropertyNode->GetID());
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

  vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
    d->DisplayNodeSelector->currentNode());

  if (displayNode)
  {
    displayNode->SetAndObserveROINodeID(roiNode->GetID());
  }
}
  

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidget::createVolumeRenderingDisplayNode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkSlicerVolumeRenderingLogic *logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  vtkMRMLVolumeRenderingDisplayNode *displayNode = logic->CreateVolumeRenderingDisplayNode();

  //vtkMRMLVolumePropertyNode *propNode = vtkMRMLVolumePropertyNode::SafeDownCast(d->VolumePropertyNodeSelector->currentNode());
  vtkMRMLVolumePropertyNode *propNode = NULL;

  //vtkMRMLROINode            *roiNode = vtkMRMLROINode::SafeDownCast(d->ROINodeSelector->currentNode());
  vtkMRMLROINode            *roiNode = NULL;

  logic->UpdateDisplayNodeFromVolumeNode(displayNode,
                                            vtkMRMLVolumeNode::SafeDownCast(d->ActiveVolumeNodeSelector->currentNode()),
                                            &propNode, &roiNode);

  setMRMLDisplayNode(displayNode);
  //d->DisplayNodeSelector->setCurrentNode(displayNode);
  //d->VolumePropertyNodeSelector->setCurrentNode(propNode);
  //d->ROINodeSelector->setCurrentNode(roiNode);
  return displayNode;
}
