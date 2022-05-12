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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

  ==============================================================================*/

// qMRML includes
#include "qMRMLMarkupsPlaneWidget.h"
#include "ui_qMRMLMarkupsPlaneWidget.h"

// MRML includes
#include <vtkMRMLMarkupsPlaneDisplayNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>

// STD includes
#include <vector>

// --------------------------------------------------------------------------
class qMRMLMarkupsPlaneWidgetPrivate:
  public Ui_qMRMLMarkupsPlaneWidget
{
public:
  qMRMLMarkupsPlaneWidgetPrivate(qMRMLMarkupsPlaneWidget& object);
  void setupUi(qMRMLMarkupsPlaneWidget* widget);

  const char* getPlaneTypeName(int planeType);

protected:
  qMRMLMarkupsPlaneWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qMRMLMarkupsPlaneWidget);
};

// --------------------------------------------------------------------------
qMRMLMarkupsPlaneWidgetPrivate::qMRMLMarkupsPlaneWidgetPrivate(qMRMLMarkupsPlaneWidget& widget)
  : q_ptr(&widget)
{
}

// --------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidgetPrivate::setupUi(qMRMLMarkupsPlaneWidget* widget)
{
  Q_Q(qMRMLMarkupsPlaneWidget);

  this->Ui_qMRMLMarkupsPlaneWidget::setupUi(widget);

  this->planeTypeComboBox->clear();
  for (int planeType = 0; planeType < vtkMRMLMarkupsPlaneNode::PlaneType_Last; ++planeType)
    {
    this->planeTypeComboBox->addItem(this->getPlaneTypeName(planeType), planeType);
    }

  this->planeSizeModeComboBox->clear();
  for (int sizeMode = 0; sizeMode < vtkMRMLMarkupsPlaneNode::SizeMode_Last; ++sizeMode)
    {
    this->planeSizeModeComboBox->addItem(vtkMRMLMarkupsPlaneNode::GetSizeModeAsString(sizeMode), sizeMode);
    }

  QObject::connect(this->planeTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onPlaneTypeIndexChanged()));
  QObject::connect(this->planeSizeModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onPlaneSizeModeIndexChanged()));

  QObject::connect(this->sizeXSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneSizeSpinBoxChanged()));
  QObject::connect(this->sizeYSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneSizeSpinBoxChanged()));


  QObject::connect(this->boundsXMinSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));
  QObject::connect(this->boundsXMaxSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));
  QObject::connect(this->boundsYMinSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));
  QObject::connect(this->boundsYMaxSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(onPlaneBoundsSpinBoxChanged()));

  QObject::connect(this->normalVisibilityCheckBox, SIGNAL(stateChanged(int)), q, SLOT(onNormalVisibilityCheckBoxChanged()));
  QObject::connect(this->normalOpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(onNormalOpacitySliderChanged()));

  q->setEnabled(vtkMRMLMarkupsPlaneNode::SafeDownCast(q->MarkupsNode) != nullptr);
  q->setVisible(vtkMRMLMarkupsPlaneNode::SafeDownCast(q->MarkupsNode) != nullptr);
}

// --------------------------------------------------------------------------
const char* qMRMLMarkupsPlaneWidgetPrivate::getPlaneTypeName(int planeType)
{
  switch (planeType)
    {
    case vtkMRMLMarkupsPlaneNode::PlaneType3Points:
      return "Three points";
    case vtkMRMLMarkupsPlaneNode::PlaneTypePointNormal:
      return "Point normal";
    case vtkMRMLMarkupsPlaneNode::PlaneTypePlaneFit:
      return "Plane fit";
    default:
      break;
    }
  return "";
}

// --------------------------------------------------------------------------
// qMRMLMarkupsPlaneWidget methods

// --------------------------------------------------------------------------
qMRMLMarkupsPlaneWidget::qMRMLMarkupsPlaneWidget(QWidget* parent)
  : Superclass(parent), d_ptr(new qMRMLMarkupsPlaneWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qMRMLMarkupsPlaneWidget::~qMRMLMarkupsPlaneWidget() = default;

// --------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::setup()
{
  Q_D(qMRMLMarkupsPlaneWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkMRMLMarkupsPlaneNode* qMRMLMarkupsPlaneWidget::mrmlPlaneNode()const
{
  Q_D(const qMRMLMarkupsPlaneWidget);
  return vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  this->qvtkReconnect(this->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromMRML()));

  this->MarkupsNode = markupsNode;
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLMarkupsPlaneWidget);

  this->setEnabled(this->canManageMRMLMarkupsNode(this->MarkupsNode));
  this->setVisible(this->canManageMRMLMarkupsNode(this->MarkupsNode));

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }

  bool wasBlocked = d->planeTypeComboBox->blockSignals(true);
  d->planeTypeComboBox->setCurrentIndex(d->planeTypeComboBox->findData(planeNode->GetPlaneType()));
  d->planeTypeComboBox->blockSignals(wasBlocked);

  wasBlocked = d->planeSizeModeComboBox->blockSignals(true);
  d->planeSizeModeComboBox->setCurrentIndex(d->planeSizeModeComboBox->findData(planeNode->GetSizeMode()));
  d->planeSizeModeComboBox->blockSignals(wasBlocked);

  double* size = planeNode->GetSize();

  wasBlocked = d->sizeXSpinBox->blockSignals(true);
  d->sizeXSpinBox->setValue(size[0]);
  d->sizeXSpinBox->blockSignals(wasBlocked);
  d->sizeXSpinBox->setEnabled(planeNode->GetSizeMode() != vtkMRMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->sizeYSpinBox->blockSignals(true);
  d->sizeYSpinBox->setValue(size[1]);
  d->sizeYSpinBox->blockSignals(wasBlocked);
  d->sizeYSpinBox->setEnabled(planeNode->GetSizeMode() != vtkMRMLMarkupsPlaneNode::SizeModeAuto);

  double* bounds = planeNode->GetPlaneBounds();

  wasBlocked = d->boundsXMinSpinBox->blockSignals(true);
  d->boundsXMinSpinBox->setValue(bounds[0]);
  d->boundsXMinSpinBox->blockSignals(wasBlocked);
  d->boundsXMinSpinBox->setEnabled(planeNode->GetSizeMode() != vtkMRMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->boundsXMaxSpinBox->blockSignals(true);
  d->boundsXMaxSpinBox->setValue(bounds[1]);
  d->boundsXMaxSpinBox->blockSignals(wasBlocked);
  d->boundsXMaxSpinBox->setEnabled(planeNode->GetSizeMode() != vtkMRMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->boundsYMinSpinBox->blockSignals(true);
  d->boundsYMinSpinBox->setValue(bounds[2]);
  d->boundsYMinSpinBox->blockSignals(wasBlocked);
  d->boundsYMinSpinBox->setEnabled(planeNode->GetSizeMode() != vtkMRMLMarkupsPlaneNode::SizeModeAuto);

  wasBlocked = d->boundsYMaxSpinBox->blockSignals(true);
  d->boundsYMaxSpinBox->setValue(bounds[3]);
  d->boundsYMaxSpinBox->blockSignals(wasBlocked);
  d->boundsYMaxSpinBox->setEnabled(planeNode->GetSizeMode() != vtkMRMLMarkupsPlaneNode::SizeModeAuto);

  vtkMRMLMarkupsPlaneDisplayNode* planeDisplayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(planeNode->GetDisplayNode());
  if (planeDisplayNode)
    {
    wasBlocked = d->normalVisibilityCheckBox->blockSignals(true);
    d->normalVisibilityCheckBox->setChecked(planeDisplayNode->GetNormalVisibility());
    d->normalVisibilityCheckBox->blockSignals(wasBlocked);

    wasBlocked = d->normalOpacitySlider->blockSignals(true);
    d->normalOpacitySlider->setValue(planeDisplayNode->GetNormalOpacity());
    d->normalOpacitySlider->blockSignals(wasBlocked);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::onPlaneTypeIndexChanged()
{
  Q_D(qMRMLMarkupsPlaneWidget);
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }
  planeNode->SetPlaneType(d->planeTypeComboBox->currentData().toInt());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::onPlaneSizeModeIndexChanged()
{
  Q_D(qMRMLMarkupsPlaneWidget);
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }
  planeNode->SetSizeMode(d->planeSizeModeComboBox->currentData().toInt());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::onPlaneSizeSpinBoxChanged()
{
  Q_D(qMRMLMarkupsPlaneWidget);
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }
  planeNode->SetSize(d->sizeXSpinBox->value(), d->sizeYSpinBox->value());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::onPlaneBoundsSpinBoxChanged()
{
  Q_D(qMRMLMarkupsPlaneWidget);
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
  {
    return;
  }
  double xMin = std::min(d->boundsXMinSpinBox->value(), d->boundsXMaxSpinBox->value());
  double xMax = std::max(d->boundsXMinSpinBox->value(), d->boundsXMaxSpinBox->value());
  double yMin = std::min(d->boundsYMinSpinBox->value(), d->boundsYMaxSpinBox->value());
  double yMax = std::max(d->boundsYMinSpinBox->value(), d->boundsYMaxSpinBox->value());
  planeNode->SetPlaneBounds(xMin, xMax, yMin, yMax);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::onNormalVisibilityCheckBoxChanged()
{
  Q_D(qMRMLMarkupsPlaneWidget);
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }

  vtkMRMLMarkupsPlaneDisplayNode* displayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(planeNode->GetDisplayNode());
  if (!displayNode)
    {
    return;
    }

  displayNode->SetNormalVisibility(d->normalVisibilityCheckBox->checkState() == Qt::Checked);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsPlaneWidget::onNormalOpacitySliderChanged()
{
  Q_D(qMRMLMarkupsPlaneWidget);
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->MarkupsNode);
  if (!planeNode)
    {
    return;
    }

  vtkMRMLMarkupsPlaneDisplayNode* displayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(planeNode->GetDisplayNode());
  if (!displayNode)
    {
    return;
    }

  displayNode->SetNormalOpacity(d->normalOpacitySlider->value());
}

//-----------------------------------------------------------------------------
bool qMRMLMarkupsPlaneWidget::canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const
{
  Q_D(const qMRMLMarkupsPlaneWidget);

  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(markupsNode);
  if (!planeNode)
    {
    return false;
    }

  return true;
}
