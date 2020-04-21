/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QButtonGroup>

// qMRML includes
#include "qMRMLClipNodeWidget.h"
#include "ui_qMRMLClipNodeWidget.h"

// MRML includes
#include <vtkMRMLClipModelsNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLClipNodeWidgetPrivate: public Ui_qMRMLClipNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLClipNodeWidget);

protected:
  qMRMLClipNodeWidget* const q_ptr;

public:
  qMRMLClipNodeWidgetPrivate(qMRMLClipNodeWidget& object);
  void init();

  vtkSmartPointer<vtkMRMLClipModelsNode> MRMLClipNode;
  bool                                   IsUpdatingWidgetFromMRML;
};

//------------------------------------------------------------------------------
qMRMLClipNodeWidgetPrivate::qMRMLClipNodeWidgetPrivate(qMRMLClipNodeWidget& object)
  : q_ptr(&object)
{
  this->IsUpdatingWidgetFromMRML = false;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidgetPrivate::init()
{
  Q_Q(qMRMLClipNodeWidget);
  this->setupUi(q);

  QButtonGroup* clipTypeGroup = new QButtonGroup(q);
  clipTypeGroup->addButton(this->UnionRadioButton);
  clipTypeGroup->addButton(this->IntersectionRadioButton);

  QButtonGroup* redSliceClipStateGroup = new QButtonGroup(q);
  redSliceClipStateGroup->addButton(this->RedPositiveRadioButton);
  redSliceClipStateGroup->addButton(this->RedNegativeRadioButton);

  QButtonGroup* yellowSliceClipStateGroup = new QButtonGroup(q);
  yellowSliceClipStateGroup->addButton(this->YellowPositiveRadioButton);
  yellowSliceClipStateGroup->addButton(this->YellowNegativeRadioButton);

  QButtonGroup* greenSliceClipStateGroup = new QButtonGroup(q);
  greenSliceClipStateGroup->addButton(this->GreenPositiveRadioButton);
  greenSliceClipStateGroup->addButton(this->GreenNegativeRadioButton);

  QObject::connect(this->UnionRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeClipType()));
  QObject::connect(this->IntersectionRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeClipType()));

  QObject::connect(this->RedSliceClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeRedClipState()));
  QObject::connect(this->RedPositiveRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeRedClipState()));
  QObject::connect(this->RedNegativeRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeRedClipState()));

  QObject::connect(this->YellowSliceClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeYellowClipState()));
  QObject::connect(this->YellowPositiveRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeYellowClipState()));
  QObject::connect(this->YellowNegativeRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeYellowClipState()));

  QObject::connect(this->GreenSliceClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeGreenClipState()));
  QObject::connect(this->GreenPositiveRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeGreenClipState()));
  QObject::connect(this->GreenNegativeRadioButton, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeGreenClipState()));

  QObject::connect(this->WholeCellClippingCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(updateNodeClippingMethod()));


  q->setEnabled(this->MRMLClipNode.GetPointer() != nullptr);
}

//------------------------------------------------------------------------------
qMRMLClipNodeWidget::qMRMLClipNodeWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLClipNodeWidgetPrivate(*this))
{
  Q_D(qMRMLClipNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLClipNodeWidget::~qMRMLClipNodeWidget() = default;

//------------------------------------------------------------------------------
vtkMRMLClipModelsNode* qMRMLClipNodeWidget::mrmlClipNode()const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->MRMLClipNode;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setMRMLClipNode(vtkMRMLNode* node)
{
  this->setMRMLClipNode(vtkMRMLClipModelsNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setMRMLClipNode(vtkMRMLClipModelsNode* clipNode)
{
  Q_D(qMRMLClipNodeWidget);
  qvtkReconnect(d->MRMLClipNode, clipNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->MRMLClipNode = clipNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setClipType(int type)
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode.GetPointer())
    {
    return;
    }
  d->MRMLClipNode->SetClipType(type);
}

//------------------------------------------------------------------------------
int qMRMLClipNodeWidget::clipType()const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->UnionRadioButton->isChecked() ?
    vtkMRMLClipModelsNode::ClipUnion :
    vtkMRMLClipModelsNode::ClipIntersection;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setRedSliceClipState(int state)
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode.GetPointer())
    {
    return;
    }
  d->MRMLClipNode->SetRedSliceClipState(state);
}

//------------------------------------------------------------------------------
int qMRMLClipNodeWidget::redSliceClipState()const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->RedSliceClippingCheckBox->isChecked() ?
    (d->RedPositiveRadioButton->isChecked() ?
      vtkMRMLClipModelsNode::ClipPositiveSpace :
      vtkMRMLClipModelsNode::ClipNegativeSpace) :
    vtkMRMLClipModelsNode::ClipOff;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setYellowSliceClipState(int state)
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode.GetPointer())
    {
    return;
    }
  d->MRMLClipNode->SetYellowSliceClipState(state);
}

//------------------------------------------------------------------------------
int qMRMLClipNodeWidget::yellowSliceClipState()const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->YellowSliceClippingCheckBox->isChecked() ?
    (d->YellowPositiveRadioButton->isChecked() ?
      vtkMRMLClipModelsNode::ClipPositiveSpace :
      vtkMRMLClipModelsNode::ClipNegativeSpace) :
    vtkMRMLClipModelsNode::ClipOff;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setGreenSliceClipState(int state)
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode.GetPointer())
    {
    return;
    }
  d->MRMLClipNode->SetGreenSliceClipState(state);
}

//------------------------------------------------------------------------------
int qMRMLClipNodeWidget::greenSliceClipState()const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->GreenSliceClippingCheckBox->isChecked() ?
    (d->GreenPositiveRadioButton->isChecked() ?
      vtkMRMLClipModelsNode::ClipPositiveSpace :
      vtkMRMLClipModelsNode::ClipNegativeSpace) :
    vtkMRMLClipModelsNode::ClipOff;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::setClippingMethod(vtkMRMLClipModelsNode::ClippingMethodType state)
{
  Q_D(qMRMLClipNodeWidget);
  if (!d->MRMLClipNode.GetPointer())
    {
    return;
    }
  d->MRMLClipNode->SetClippingMethod(state);
}

//------------------------------------------------------------------------------
vtkMRMLClipModelsNode::ClippingMethodType qMRMLClipNodeWidget::clippingMethod()const
{
  Q_D(const qMRMLClipNodeWidget);
  return d->WholeCellClippingCheckBox->isChecked() ? vtkMRMLClipModelsNode::WholeCells : vtkMRMLClipModelsNode::Straight;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLClipNodeWidget);
  this->setEnabled(d->MRMLClipNode.GetPointer() != nullptr);
  if (d->MRMLClipNode.GetPointer() == nullptr)
    {
    return;
    }
  bool oldUpdating = d->IsUpdatingWidgetFromMRML;
  d->IsUpdatingWidgetFromMRML = true;

  d->UnionRadioButton->setChecked(
    d->MRMLClipNode->GetClipType() == vtkMRMLClipModelsNode::ClipUnion);
  d->IntersectionRadioButton->setChecked(
    d->MRMLClipNode->GetClipType() == vtkMRMLClipModelsNode::ClipIntersection);

  // Setting one checkbox might trigger a signal which result to action in an unstable state
  // to be a valid state, all the checkboxes need to be set.
  d->RedSliceClippingCheckBox->setChecked(
    d->MRMLClipNode->GetRedSliceClipState() != vtkMRMLClipModelsNode::ClipOff);
  d->RedPositiveRadioButton->setChecked(
    d->MRMLClipNode->GetRedSliceClipState() == vtkMRMLClipModelsNode::ClipPositiveSpace);
  d->RedNegativeRadioButton->setChecked(
    d->MRMLClipNode->GetRedSliceClipState() == vtkMRMLClipModelsNode::ClipNegativeSpace);

  d->YellowSliceClippingCheckBox->setChecked(
    d->MRMLClipNode->GetYellowSliceClipState() != vtkMRMLClipModelsNode::ClipOff);
  d->YellowPositiveRadioButton->setChecked(
    d->MRMLClipNode->GetYellowSliceClipState() == vtkMRMLClipModelsNode::ClipPositiveSpace);
  d->YellowNegativeRadioButton->setChecked(
    d->MRMLClipNode->GetYellowSliceClipState() == vtkMRMLClipModelsNode::ClipNegativeSpace);

  d->GreenSliceClippingCheckBox->setChecked(
    d->MRMLClipNode->GetGreenSliceClipState() != vtkMRMLClipModelsNode::ClipOff);
  d->GreenPositiveRadioButton->setChecked(
    d->MRMLClipNode->GetGreenSliceClipState() == vtkMRMLClipModelsNode::ClipPositiveSpace);
  d->GreenNegativeRadioButton->setChecked(
    d->MRMLClipNode->GetGreenSliceClipState() == vtkMRMLClipModelsNode::ClipNegativeSpace);

  d->WholeCellClippingCheckBox->setChecked(
    d->MRMLClipNode->GetClippingMethod() != vtkMRMLClipModelsNode::Straight);

  d->IsUpdatingWidgetFromMRML = oldUpdating;
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateNodeClipType()
{
  this->setClipType(this->clipType());
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateNodeRedClipState()
{
  Q_D(const qMRMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }
  this->setRedSliceClipState(this->redSliceClipState());
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateNodeYellowClipState()
{
  Q_D(const qMRMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }
  this->setYellowSliceClipState(this->yellowSliceClipState());
}

//------------------------------------------------------------------------------
void qMRMLClipNodeWidget::updateNodeGreenClipState()
{
  Q_D(const qMRMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }
  this->setGreenSliceClipState(this->greenSliceClipState());
}

void qMRMLClipNodeWidget::updateNodeClippingMethod()
{
  Q_D(const qMRMLClipNodeWidget);
  if (d->IsUpdatingWidgetFromMRML)
    {
    return;
    }
  this->setClippingMethod(this->clippingMethod());
}
