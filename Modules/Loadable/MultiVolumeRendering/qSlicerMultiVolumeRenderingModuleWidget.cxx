/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerMultiVolumeRenderingModuleWidget.h"
#include "ui_qSlicerMultiVolumeRenderingModuleWidget.h"
#include "vtkMRMLMultiVolumeRenderingDisplayNode.h"
#include "vtkSlicerMultiVolumeRenderingLogic.h"

// MRML includes
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVector.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <cassert>
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_MultiVolumeRendering
class qSlicerMultiVolumeRenderingModuleWidgetPrivate: public Ui_qSlicerMultiVolumeRenderingModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerMultiVolumeRenderingModuleWidget);

protected:
  qSlicerMultiVolumeRenderingModuleWidget* const q_ptr;

public:
  qSlicerMultiVolumeRenderingModuleWidgetPrivate(qSlicerMultiVolumeRenderingModuleWidget& object);
  virtual void setupUi(qSlicerMultiVolumeRenderingModuleWidget*);

  vtkMRMLMultiVolumeRenderingDisplayNode* createNewDisplayNode(vtkMRMLVolumeNode* bg, vtkMRMLVolumeNode* fg, vtkMRMLVolumeNode* label);

  vtkMRMLMultiVolumeRenderingDisplayNode* DisplayNode;

};

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModuleWidgetPrivate::qSlicerMultiVolumeRenderingModuleWidgetPrivate(
  qSlicerMultiVolumeRenderingModuleWidget& object)
  :q_ptr(&object)
{
  this->DisplayNode = NULL;
}

void qSlicerMultiVolumeRenderingModuleWidgetPrivate::setupUi(qSlicerMultiVolumeRenderingModuleWidget* q)
{
  this->Ui_qSlicerMultiVolumeRenderingModule::setupUi(q);

  QObject::connect(this->MRMLNodeComboBoxBackgroundVolume,
                    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                    q, SLOT(onCurrentBgVolumeMRMLImageNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->MRMLNodeComboBoxForegroundVolume,
                    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                    q, SLOT(onCurrentFgVolumeMRMLImageNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->MRMLNodeComboBoxLabelmapVolume,
                    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                    q, SLOT(onCurrentLabelmapVolumeMRMLImageNodeChanged(vtkMRMLNode*)));

  // Inputs
  QObject::connect(this->CheckBoxBgVolume,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onVisibilityBgVolumeChanged(bool)));

  QObject::connect(this->CheckBoxFgVolume,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onVisibilityFgVolumeChanged(bool)));

  QObject::connect(this->CheckBoxLabelmapVolume,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onVisibilityLabelmapVolumeChanged(bool)));

  QObject::connect(this->MRMLNodeComboBoxDisplayNode,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLDisplayNodeChanged(vtkMRMLNode*)));


}

vtkMRMLMultiVolumeRenderingDisplayNode* qSlicerMultiVolumeRenderingModuleWidgetPrivate
::createNewDisplayNode(vtkMRMLVolumeNode* bg, vtkMRMLVolumeNode* fg, vtkMRMLVolumeNode* label)
{
  Q_Q(qSlicerMultiVolumeRenderingModuleWidget);

  vtkSlicerMultiVolumeRenderingLogic *logic =
    vtkSlicerMultiVolumeRenderingLogic::SafeDownCast(q->logic());

  vtkMRMLMultiVolumeRenderingDisplayNode *displayNode =
    logic->CreateDisplayNode();

  if (bg)
   bg->AddAndObserveDisplayNodeID(displayNode->GetID());

  if (fg)
   fg->AddAndObserveDisplayNodeID(displayNode->GetID());

  if (label)
   label->AddAndObserveDisplayNodeID(displayNode->GetID());

  return displayNode;
}

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModuleWidget::qSlicerMultiVolumeRenderingModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerMultiVolumeRenderingModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModuleWidget::~qSlicerMultiVolumeRenderingModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::setup()
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

// ---------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::onVisibilityBgVolumeChanged(bool visible)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  d->DisplayNode->SetBgVisibility(visible);
  d->DisplayNode->UpdateVisibility();
}

// ---------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::onVisibilityFgVolumeChanged(bool visible)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  d->DisplayNode->SetFgVisibility(visible);
  d->DisplayNode->UpdateVisibility();
}

// ---------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::onVisibilityLabelmapVolumeChanged(bool visible)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  d->DisplayNode->SetLabelmapVisibility(visible);
  d->DisplayNode->UpdateVisibility();
}

// ---------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::onCurrentBgVolumeMRMLImageNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);

  vtkMRMLScalarVolumeNode* bgVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  if (!bgVolumeNode)
  {
    return;
  }

  vtkMRMLScalarVolumeNode* fgVolumeNode = NULL;
  vtkMRMLLabelMapVolumeNode* labelmapVolumeNode = NULL;

  if (d->DisplayNode)
  {
    fgVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->DisplayNode->GetFgVolumeNode());
    labelmapVolumeNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(d->DisplayNode->GetLabelmapVolumeNode());
  }

  vtkSlicerMultiVolumeRenderingLogic *logic =
    vtkSlicerMultiVolumeRenderingLogic::SafeDownCast(this->logic());

  vtkMRMLMultiVolumeRenderingDisplayNode *dnode = logic->FindFirstMatchedDisplayNode(bgVolumeNode, fgVolumeNode, labelmapVolumeNode);

  if (!this->mrmlScene()->IsClosing())
  {
    if (!dnode)
      dnode = d->createNewDisplayNode(bgVolumeNode, fgVolumeNode, labelmapVolumeNode);
    else
      dnode->Modified();
  }

  this->setMRMLDisplayNode(dnode);
}

//----------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::onCurrentFgVolumeMRMLImageNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);

  vtkMRMLScalarVolumeNode* fgVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  if (!fgVolumeNode)
  {
    return;
  }

  vtkMRMLScalarVolumeNode* bgVolumeNode = NULL;
  vtkMRMLLabelMapVolumeNode* labelmapVolumeNode = NULL;

  if (d->DisplayNode)
  {
    bgVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->DisplayNode->GetBgVolumeNode());
    labelmapVolumeNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(d->DisplayNode->GetLabelmapVolumeNode());
  }

  vtkSlicerMultiVolumeRenderingLogic *logic =
    vtkSlicerMultiVolumeRenderingLogic::SafeDownCast(this->logic());

  vtkMRMLMultiVolumeRenderingDisplayNode *dnode = logic->FindFirstMatchedDisplayNode(bgVolumeNode, fgVolumeNode, labelmapVolumeNode);

  if (!this->mrmlScene()->IsClosing())
  {
    if (!dnode)
      dnode = d->createNewDisplayNode(bgVolumeNode, fgVolumeNode, labelmapVolumeNode);
    else
      dnode->Modified();
  }

  this->setMRMLDisplayNode(dnode);
}

//----------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::onCurrentLabelmapVolumeMRMLImageNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);

  vtkMRMLLabelMapVolumeNode* labelmapVolumeNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(node);

  if (!labelmapVolumeNode)
  {
    return;
  }

  vtkMRMLScalarVolumeNode* fgVolumeNode = NULL;
  vtkMRMLScalarVolumeNode* bgVolumeNode = NULL;

  if (d->DisplayNode)
  {
    fgVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->DisplayNode->GetFgVolumeNode());
    bgVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(d->DisplayNode->GetBgVolumeNode());
  }

  vtkSlicerMultiVolumeRenderingLogic *logic =
    vtkSlicerMultiVolumeRenderingLogic::SafeDownCast(this->logic());

  vtkMRMLMultiVolumeRenderingDisplayNode *dnode = logic->FindFirstMatchedDisplayNode(bgVolumeNode, fgVolumeNode, labelmapVolumeNode);

  if (!this->mrmlScene()->IsClosing())
  {
    if (!dnode)
      dnode = d->createNewDisplayNode(bgVolumeNode, fgVolumeNode, labelmapVolumeNode);
    else
      dnode->Modified();
  }

  this->setMRMLDisplayNode(dnode);
}

//----------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::setMRMLDisplayNode(vtkMRMLNode* displayNode)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);
  d->MRMLNodeComboBoxDisplayNode->setCurrentNode(displayNode);
}

// ---------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::onCurrentMRMLDisplayNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);

  vtkMRMLMultiVolumeRenderingDisplayNode* displayNode =
    vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(node);

  // update view node references
  vtkMRMLScalarVolumeNode* bgVolumeNode = this->getCurrentBgVolumeNode();
  vtkMRMLScalarVolumeNode* fgVolumeNode = this->getCurrentFgVolumeNode();
  vtkMRMLLabelMapVolumeNode* labelmapVolumeNode = this->getCurrentLabelmapVolumeNode();

  // if display node is not referenced by current volume, add the refrence
  if (bgVolumeNode && displayNode)
  {
    vtkSlicerMultiVolumeRenderingLogic *logic =
      vtkSlicerMultiVolumeRenderingLogic::SafeDownCast(this->logic());

    vtkMRMLMultiVolumeRenderingDisplayNode* dnode =
      logic->GetDisplayNodeByID(bgVolumeNode, displayNode->GetID());

    if (dnode != displayNode)
    {
      bgVolumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }
  }

  if (fgVolumeNode && displayNode)
  {
    vtkSlicerMultiVolumeRenderingLogic *logic =
      vtkSlicerMultiVolumeRenderingLogic::SafeDownCast(this->logic());

    vtkMRMLMultiVolumeRenderingDisplayNode* dnode =
      logic->GetDisplayNodeByID(fgVolumeNode, displayNode->GetID());

    if (dnode != displayNode)
    {
      fgVolumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }
  }

  if (labelmapVolumeNode && displayNode)
  {
    vtkSlicerMultiVolumeRenderingLogic *logic =
      vtkSlicerMultiVolumeRenderingLogic::SafeDownCast(this->logic());

    vtkMRMLMultiVolumeRenderingDisplayNode* dnode =
      logic->GetDisplayNodeByID(labelmapVolumeNode, displayNode->GetID());

    if (dnode != displayNode)
    {
      labelmapVolumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }
  }

  this->qvtkReconnect(d->DisplayNode, displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateRenderingFromMRMLDisplayNode()));

  d->DisplayNode = displayNode;

  this->updateRenderingFromMRMLDisplayNode();
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qSlicerMultiVolumeRenderingModuleWidget::getCurrentBgVolumeNode()const
{
  Q_D(const qSlicerMultiVolumeRenderingModuleWidget);
  return vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBoxBackgroundVolume->currentNode());
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qSlicerMultiVolumeRenderingModuleWidget::getCurrentFgVolumeNode()const
{
  Q_D(const qSlicerMultiVolumeRenderingModuleWidget);
  return vtkMRMLScalarVolumeNode::SafeDownCast(d->MRMLNodeComboBoxForegroundVolume->currentNode());
}

// --------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode* qSlicerMultiVolumeRenderingModuleWidget::getCurrentLabelmapVolumeNode()const
{
  Q_D(const qSlicerMultiVolumeRenderingModuleWidget);
  return vtkMRMLLabelMapVolumeNode::SafeDownCast(d->MRMLNodeComboBoxLabelmapVolume->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModuleWidget::updateRenderingFromMRMLDisplayNode()
{
  Q_D(qSlicerMultiVolumeRenderingModuleWidget);
}
