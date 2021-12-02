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

// Qt includes

// qMRML includes
#include "qMRMLMarkupsInteractionHandleWidget.h"
#include "ui_qMRMLMarkupsInteractionHandleWidget.h"

// MRML includes
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsROIDisplayNode.h>

// --------------------------------------------------------------------------
class qMRMLMarkupsInteractionHandleWidgetPrivate: public Ui_qMRMLMarkupsInteractionHandleWidget
{
  Q_DECLARE_PUBLIC(qMRMLMarkupsInteractionHandleWidget);
protected:
  qMRMLMarkupsInteractionHandleWidget* const q_ptr;
public:
  qMRMLMarkupsInteractionHandleWidgetPrivate(qMRMLMarkupsInteractionHandleWidget& object);
  void init();

  vtkMRMLMarkupsDisplayNode* DisplayNode;
};

// --------------------------------------------------------------------------
qMRMLMarkupsInteractionHandleWidgetPrivate::qMRMLMarkupsInteractionHandleWidgetPrivate(qMRMLMarkupsInteractionHandleWidget& object)
  : q_ptr(&object)
{
  this->DisplayNode = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsInteractionHandleWidgetPrivate::init()
{
  Q_Q(qMRMLMarkupsInteractionHandleWidget);
  this->setupUi(q);
  q->setEnabled(this->DisplayNode != nullptr);

  QObject::connect(this->overallVisibilityCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->translateVisibilityCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->rotateVisibilityCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->scaleVisibilityCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->interactionHandleScaleSlider, SIGNAL(valueChanged(double)), q, SLOT(updateMRMLFromWidget()));
}

// --------------------------------------------------------------------------
// qMRMLMarkupsInteractionHandleWidget methods

// --------------------------------------------------------------------------
qMRMLMarkupsInteractionHandleWidget::qMRMLMarkupsInteractionHandleWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLMarkupsInteractionHandleWidgetPrivate(*this))
{
  Q_D(qMRMLMarkupsInteractionHandleWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLMarkupsInteractionHandleWidget::~qMRMLMarkupsInteractionHandleWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* qMRMLMarkupsInteractionHandleWidget::mrmlDisplayNode() const
{
  Q_D(const qMRMLMarkupsInteractionHandleWidget);
  return d->DisplayNode;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsInteractionHandleWidget::setMRMLDisplayNode(vtkMRMLMarkupsDisplayNode* displayNode)
{
  Q_D(qMRMLMarkupsInteractionHandleWidget);
  this->qvtkReconnect(d->DisplayNode, displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));

  d->DisplayNode = displayNode;

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLMarkupsInteractionHandleWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLMarkupsInteractionHandleWidget);
  this->setEnabled(d->DisplayNode != nullptr);
  d->overallVisibilityCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->translateVisibilityCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->rotateVisibilityCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->scaleVisibilityCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->interactionHandleScaleSlider->setEnabled(d->DisplayNode != nullptr);
  if (!d->DisplayNode)
    {
    return;
    }

  // Scale handles currently not implemented for representations other than ROI
  // Disable by default.
  if (!vtkMRMLMarkupsROIDisplayNode::SafeDownCast(d->DisplayNode))
    {
    // Scale handles not currently implemented for non ROI nodes
    d->scaleVisibilityCheckBox->setVisible(false);
    }
  else
    {
    d->scaleVisibilityCheckBox->setVisible(true);
    }

  bool wasBlocking = false;

  // Interactive Mode
  wasBlocking = d->overallVisibilityCheckBox->blockSignals(true);
  d->overallVisibilityCheckBox->setChecked(d->DisplayNode->GetHandlesInteractive());
  d->overallVisibilityCheckBox->blockSignals(wasBlocking);


  wasBlocking = d->translateVisibilityCheckBox->blockSignals(true);
  d->translateVisibilityCheckBox->setChecked(d->DisplayNode->GetTranslationHandleVisibility());
  d->translateVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->rotateVisibilityCheckBox->blockSignals(true);
  d->rotateVisibilityCheckBox->setChecked(d->DisplayNode->GetRotationHandleVisibility());
  d->rotateVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->scaleVisibilityCheckBox->blockSignals(true);
  d->scaleVisibilityCheckBox->setChecked(d->DisplayNode->GetScaleHandleVisibility());
  d->scaleVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->interactionHandleScaleSlider->blockSignals(true);
  if (d->DisplayNode->GetInteractionHandleScale() > d->interactionHandleScaleSlider->maximum())
    {
    d->interactionHandleScaleSlider->setMaximum(d->DisplayNode->GetInteractionHandleScale());
    }
  d->interactionHandleScaleSlider->setValue(d->DisplayNode->GetInteractionHandleScale());
  wasBlocking = d->interactionHandleScaleSlider->blockSignals(wasBlocking);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsInteractionHandleWidget::updateMRMLFromWidget()
{
  Q_D(qMRMLMarkupsInteractionHandleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  MRMLNodeModifyBlocker displayNodeBlocker(d->DisplayNode);
  d->DisplayNode->SetHandlesInteractive(d->overallVisibilityCheckBox->isChecked());
  d->DisplayNode->SetTranslationHandleVisibility(d->translateVisibilityCheckBox->isChecked());
  d->DisplayNode->SetRotationHandleVisibility(d->rotateVisibilityCheckBox->isChecked());
  d->DisplayNode->SetScaleHandleVisibility(d->scaleVisibilityCheckBox->isChecked());
  d->DisplayNode->SetInteractionHandleScale(d->interactionHandleScaleSlider->value());
}
