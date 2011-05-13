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
  Q_DECLARE_PUBLIC(qSlicerVolumeRenderingModuleWidget);
protected:
  qSlicerVolumeRenderingModuleWidget* const q_ptr;

public:
  qSlicerVolumeRenderingModuleWidgetPrivate(qSlicerVolumeRenderingModuleWidget& object);
  virtual void setupUi(qSlicerVolumeRenderingModuleWidget*);
  vtkMRMLVolumeRenderingDisplayNode* createVolumeRenderingDisplayNode();

  vtkMRMLVolumeRenderingDisplayNode* DisplayNode;
};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidgetPrivate
::qSlicerVolumeRenderingModuleWidgetPrivate(
  qSlicerVolumeRenderingModuleWidget& object)
  : q_ptr(&object)
{
  this->DisplayNode = 0;
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidgetPrivate::setupUi(qSlicerVolumeRenderingModuleWidget* q)
{
  this->Ui_qSlicerVolumeRenderingModule::setupUi(q);

  QObject::connect(this->VolumeNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLVolumeNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->DisplayNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLDisplayNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->ROINodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLROINodeChanged(vtkMRMLNode*)));
  QObject::connect(this->VolumePropertyNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->ViewNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLViewNodeChanged(vtkMRMLNode*)));
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidgetPrivate
::createVolumeRenderingDisplayNode()
{
  Q_Q(qSlicerVolumeRenderingModuleWidget);
  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());

  vtkMRMLVolumeRenderingDisplayNode *displayNode =
    logic->CreateVolumeRenderingDisplayNode();

  //vtkMRMLVolumePropertyNode *propNode = vtkMRMLVolumePropertyNode::SafeDownCast(d->VolumePropertyNodeComboBox->currentNode());
  vtkMRMLVolumePropertyNode *propNode = NULL;

  //vtkMRMLROINode            *roiNode = vtkMRMLROINode::SafeDownCast(d->ROINodeComboBox->currentNode());
  vtkMRMLROINode            *roiNode = NULL;

  logic->UpdateDisplayNodeFromVolumeNode(displayNode, q->mrmlVolumeNode(),
                                         &propNode, &roiNode);

  return displayNode;
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget
::qSlicerVolumeRenderingModuleWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
    , d_ptr( new qSlicerVolumeRenderingModuleWidgetPrivate(*this) )
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
void qSlicerVolumeRenderingModuleWidget::setMRMLViewNode(vtkMRMLNode* viewNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->ViewNodeComboBox->setCurrentNode(viewNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMRMLViewNodeChanged(vtkMRMLNode* viewNode)
{
  // set view in the currently selected display node
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    if (viewNode && displayNode->IsViewNodeIDPresent(viewNode->GetID()))
      {
      return;
      }
    displayNode->RemoveAllViewNodeIDs();
    displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLDisplayNode(vtkMRMLNode* displayNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->DisplayNodeComboBox->setCurrentNode(displayNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::onCurrentMRMLDisplayNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  if (d->DisplayNode)
    {
    //d->DisplayNode->RemoveAllViewNodeIDs();
    d->DisplayNode->SetVisibility(false);
    }

  vtkMRMLVolumeRenderingDisplayNode* displayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
  d->DisplayNode = displayNode;


  // update view node references
  vtkMRMLScalarVolumeNode* volumeNode = this->mrmlVolumeNode();
  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  vtkMRMLVolumeRenderingDisplayNode* dnode = 0;

  // if display node is not referenced by current volume, add the refrence
  if (volumeNode && displayNode)
    {
    dnode = logic->GetVolumeRenderingDisplayNodeByID(volumeNode,
                                                     displayNode->GetID());
    if (dnode != displayNode)
      {
      volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
      }
    }

  vtkMRMLViewNode *viewNode = this->mrmlViewNode();

  dnode =
    logic->GetVolumeRenderingDisplayNodeForViewNode(volumeNode, viewNode);
  if (viewNode && volumeNode && dnode != displayNode)
    {
    logic->RemoveViewFromVolumeDisplayNodes(volumeNode, viewNode);
    }

  if (displayNode)
    {
    displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
    }

  // set display node from current GUI state
  this->setMRMLVolumePropertyNode(
    displayNode ? displayNode->GetVolumePropertyNode() : 0);
  this->setMRMLROINode(displayNode ? displayNode->GetROINode() : 0);

  if (displayNode)
    {
    displayNode->SetVisibility(true);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->VolumeNodeComboBox->setCurrentNode(volumeNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMRMLVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  if (!volumeNode)
    {
    this->setMRMLDisplayNode(0);
    return;
    }

  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  // see if the volume has any display node for a current viewer
  vtkMRMLViewNode *viewNode = this->mrmlViewNode();
  vtkMRMLVolumeRenderingDisplayNode *dnode =
    logic->GetVolumeRenderingDisplayNodeForViewNode(volumeNode, viewNode);
  if (!dnode)
    {
    //dnode = logic->GetFirstVolumeRenderingDisplayNode(volumeNode);
    }
  if (!dnode)
    {
    dnode = d->createVolumeRenderingDisplayNode();
    if (volumeNode)
      {
      volumeNode->AddAndObserveDisplayNodeID(dnode->GetID());
      }
    }

  if (viewNode)
    {
    // remove view from the old selected display node
    vtkMRMLVolumeRenderingDisplayNode* displayNodeOld = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
            d->DisplayNodeComboBox->currentNode());
    if (displayNodeOld)
    {
      displayNodeOld->RemoveAllViewNodeIDs();
    }

    // set view in the currently selected display node
    dnode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
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
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode* volumePropertyNode)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveVolumePropertyNodeID(
      volumePropertyNode ? volumePropertyNode->GetID() : 0);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLROINode(vtkMRMLNode* roiNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->ROINodeComboBox->setCurrentNode(roiNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMRMLROINodeChanged(vtkMRMLNode* node)
{
  vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(node);
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveROINodeID(roiNode ? roiNode->GetID() : 0);
    }
}

