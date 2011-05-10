// Qt includes
#include <QDebug>

// qMRMLWidgets include
#include "qMRMLNodeComboBox.h"

// qSlicerVolumeRendering includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModule.h"
#include "vtkSlicerVolumeRenderingLogic.h"

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLVolumeRenderingScenarioNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingModuleWidgetPrivate
  : public Ui_qSlicerVolumeRenderingModule
{
public:
  qSlicerVolumeRenderingModuleWidgetPrivate();
  virtual void setupUi(qSlicerVolumeRenderingModuleWidget*);
};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidgetPrivate::qSlicerVolumeRenderingModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidgetPrivate::setupUi(qSlicerVolumeRenderingModuleWidget* q)
{
  this->Ui_qSlicerVolumeRenderingModule::setupUi(q);

  QObject::connect(this->VolumeNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));
  QObject::connect(this->DisplayNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setMRMLDisplayNode(vtkMRMLNode*)));
  QObject::connect(this->ROINodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setMRMLROINode(vtkMRMLNode*)));
  QObject::connect(this->VolumePropertyNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setMRMLVolumePropertyNode(vtkMRMLNode*)));
  QObject::connect(this->ViewNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setMRMLViewNode(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget
::qSlicerVolumeRenderingModuleWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerVolumeRenderingModuleWidgetPrivate )
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::~qSlicerVolumeRenderingModuleWidget()
{
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qSlicerVolumeRenderingModuleWidget
::mrmlVolumeNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLScalarVolumeNode::SafeDownCast(
    d->VolumeNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidget
::mrmlDisplayNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
    d->DisplayNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
vtkMRMLViewNode* qSlicerVolumeRenderingModuleWidget::mrmlViewNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLViewNode::SafeDownCast(d->ViewNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLViewNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLViewNode *viewNodeOld = vtkMRMLViewNode::SafeDownCast(
                                  d->ViewNodeComboBox->currentNode());

  vtkMRMLViewNode *viewNode = vtkMRMLViewNode::SafeDownCast(node);

  d->ViewNodeComboBox->setCurrentNode(viewNode);

  // set view in the currently selected display node
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
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
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLVolumeRenderingDisplayNode* displayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);

  vtkMRMLVolumeRenderingDisplayNode* displayNodeOld = this->mrmlDisplayNode();

  if (displayNodeOld && displayNodeOld != displayNode)
    {
    displayNodeOld->RemoveAllViewNodeIDs();
    }

  d->DisplayNodeComboBox->setCurrentNode(displayNode);
  if (displayNode == NULL || displayNode->GetScene() == NULL)
    {
    return;
    }

  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  // set display node from current GUI state
  this->setMRMLVolumePropertyNode(
    displayNode ? displayNode->GetVolumePropertyNode() : 0);
  this->setMRMLROINode(displayNode ? displayNode->GetROINode() : 0);

  vtkMRMLScalarVolumeNode* volumeNode = this->mrmlVolumeNode();

  // update view node references
  vtkMRMLViewNode *viewNode = this->mrmlViewNode();
  vtkMRMLVolumeRenderingDisplayNode* dnode =
    logic->GetVolumeRenderingDisplayNodeForViewNode(volumeNode, viewNode);
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
    vtkMRMLVolumeRenderingDisplayNode* dnode =
      logic->GetVolumeRenderingDisplayNodeByID(volumeNode,
                                               displayNode->GetID());
    if (dnode == NULL && displayNode)
      {
      volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  d->VolumeNodeComboBox->setCurrentNode(volumeNode);

  //vtkMRMLScene* scene = volumeNode ? volumeNode->GetScene() : 0;
  //if (scene == NULL)
  //  {
  //  return;
  //  }

  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  // see if the volume has any display node for a current viewer
  vtkMRMLViewNode *viewNode = this->mrmlViewNode();
  vtkMRMLVolumeRenderingDisplayNode *dnode =
    logic->GetVolumeRenderingDisplayNodeForViewNode(volumeNode, viewNode);
  if (!dnode)
    {
    dnode = logic->GetFirstVolumeRenderingDisplayNode(volumeNode);
    }
  if (!dnode)
    {
    dnode = this->createVolumeRenderingDisplayNode();
    if (volumeNode)
      {
      volumeNode->SetAndObserveDisplayNodeID(dnode->GetID());
      }
    }

  //d->DisplayNodeComboBox->setCurrentNode(dnode);
  this->setMRMLDisplayNode(dnode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::setMRMLVolumePropertyNode(vtkMRMLNode* volumePropertyNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  // Set if not already set
  d->VolumePropertyNodeComboBox->setCurrentNode(volumePropertyNode);
  if (!volumePropertyNode || !volumePropertyNode->GetScene())
    {
    return;
    }
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveVolumePropertyNodeID(
      volumePropertyNode ?volumePropertyNode->GetID() : 0);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLROINode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(node);
  d->ROINodeComboBox->setCurrentNode(roiNode);
  if (!roiNode || !roiNode->GetScene())
    {
    return;
    }

  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveROINodeID(roiNode ? roiNode->GetID() : 0);
    }
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidget
::createVolumeRenderingDisplayNode()
{
  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  vtkMRMLVolumeRenderingDisplayNode *displayNode =
    logic->CreateVolumeRenderingDisplayNode();

  //vtkMRMLVolumePropertyNode *propNode = vtkMRMLVolumePropertyNode::SafeDownCast(d->VolumePropertyNodeComboBox->currentNode());
  vtkMRMLVolumePropertyNode *propNode = NULL;

  //vtkMRMLROINode            *roiNode = vtkMRMLROINode::SafeDownCast(d->ROINodeComboBox->currentNode());
  vtkMRMLROINode            *roiNode = NULL;

  logic->UpdateDisplayNodeFromVolumeNode(displayNode, this->mrmlVolumeNode(),
                                         &propNode, &roiNode);

  this->setMRMLDisplayNode(displayNode);
  //d->DisplayNodeComboBox->setCurrentNode(displayNode);
  //d->VolumePropertyNodeComboBox->setCurrentNode(propNode);
  //d->ROINodeComboBox->setCurrentNode(roiNode);
  return displayNode;
}
