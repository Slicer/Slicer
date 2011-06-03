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
#include "vtkMRMLAnnotationROINode.h"
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
  // Inputs
  QObject::connect(this->VisibilityCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onVisibilityChanged(bool)));
  QObject::connect(this->DisplayNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLDisplayNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->ROINodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLROINodeChanged(vtkMRMLNode*)));
  QObject::connect(this->VolumePropertyNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->ViewCheckableNodeComboBox,
                   SIGNAL(checkedNodesChanged()),
                   q, SLOT(onCheckedViewNodesChanged()));
                   
  // Rendering
  QObject::connect(this->ROICropCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onCropToggled(bool)));
  QObject::connect(this->ROIFitPushButton,
                   SIGNAL(clicked()),
                   q, SLOT(fitROIToVolume()));
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

  //vtkMRMLAnnotationROINode            *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(d->ROINodeComboBox->currentNode());
  vtkMRMLAnnotationROINode            *roiNode = NULL;

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
QList<vtkMRMLViewNode*> qSlicerVolumeRenderingModuleWidget::mrmlViewNodes()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  QList<vtkMRMLViewNode*> res;
  foreach(vtkMRMLNode* checkedNode, d->ViewCheckableNodeComboBox->checkedNodes())
    {
    res << vtkMRMLViewNode::SafeDownCast(checkedNode);
    }
  return res;
}
// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::addVolumeIntoView(vtkMRMLNode* viewNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->ViewCheckableNodeComboBox->check(viewNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCheckedViewNodesChanged()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  qDebug() << "qSlicerVolumeRenderingModuleWidget::onCheckedViewNodesChanged()";
  // set view in the currently selected display node
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  int wasModifying = displayNode->StartModify();
  displayNode->RemoveAllViewNodeIDs();
  if (!d->ViewCheckableNodeComboBox->allChecked() &&
      !d->ViewCheckableNodeComboBox->noneChecked())
    {
    foreach (vtkMRMLViewNode* viewNode, this->mrmlViewNodes())
      {
      displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
      }
    }
  else
    {
    qDebug() << "all is visible";
    }
  displayNode->EndModify(wasModifying);
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

  vtkMRMLVolumeRenderingDisplayNode* displayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);

  // update view node references
  vtkMRMLScalarVolumeNode* volumeNode = this->mrmlVolumeNode();

  // if display node is not referenced by current volume, add the refrence
  if (volumeNode && displayNode)
    {
    vtkSlicerVolumeRenderingLogic *logic =
      vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
    vtkMRMLVolumeRenderingDisplayNode* dnode =
      logic->GetVolumeRenderingDisplayNodeByID(volumeNode,
                                               displayNode->GetID());
    if (dnode != displayNode)
      {
      volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
      }
    }

  this->qvtkReconnect(d->DisplayNode, displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromMRMLDisplayNode()));

  d->DisplayNode = displayNode;

  this->updateFromMRMLDisplayNode();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::updateFromMRMLDisplayNode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  // We don't want to update ViewCheckableNodeComboBox if it's checked nodes
  // are good. Otherwise it would lead to an inconsistent state.
  if (d->DisplayNode &&
      !(d->DisplayNode->GetNumberOfViewNodeIDs() == 0 &&
      (d->ViewCheckableNodeComboBox->allChecked() ||
       d->ViewCheckableNodeComboBox->noneChecked())))
    {
    for (int i = 0; i < d->ViewCheckableNodeComboBox->nodeCount(); ++i)
      {
      vtkMRMLNode* view = d->ViewCheckableNodeComboBox->nodeFromIndex(i);
      Q_ASSERT(view);
      d->ViewCheckableNodeComboBox->setCheckState(
        view,
        d->DisplayNode && d->DisplayNode->IsViewNodeIDPresent(view->GetID()) ? Qt::Checked : Qt::Unchecked);
      }
    }

  // set display node from current GUI state
  this->setMRMLVolumePropertyNode(
    d->DisplayNode ? d->DisplayNode->GetVolumePropertyNode() : 0);
  this->setMRMLROINode(d->DisplayNode ? d->DisplayNode->GetROINode() : 0);
  d->VisibilityCheckBox->setChecked(
    d->DisplayNode ? d->DisplayNode->GetVisibility() : false);
  d->ROICropCheckBox->setChecked(
    d->DisplayNode ? d->DisplayNode->GetCroppingEnabled() : false);
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
  vtkMRMLVolumeRenderingDisplayNode *dnode =
    logic->GetFirstVolumeRenderingDisplayNode(volumeNode);
  if (!dnode)
    {
    dnode = d->createVolumeRenderingDisplayNode();
    if (volumeNode)
      {
      volumeNode->AddAndObserveDisplayNodeID(dnode->GetID());
      }
    }
  this->setMRMLDisplayNode(dnode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onVisibilityChanged(bool visible)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetVisibility(visible);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCropToggled(bool crop)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetCroppingEnabled(crop);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::fitROIToVolume()
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic())
    ->FitROIToVolume(displayNode);
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
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveROINodeID(roiNode ? roiNode->GetID() : 0);
    }
}

