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
#include <vtkMRMLMarkupsDisplayNode.h>

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
  QObject::connect(this->translateXCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->translateYCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->translateZCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->translateViewPlaneCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->rotateVisibilityCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->rotateXCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->rotateYCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->rotateZCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->scaleVisibilityCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->scaleXCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->scaleYCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->scaleZCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->scaleViewPlaneCheckBox, SIGNAL(clicked()), q, SLOT(updateMRMLFromWidget()));
  QObject::connect(this->interactionHandleScaleSlider, SIGNAL(valueChanged(double)), q, SLOT(updateMRMLFromWidget()));

  this->xLabel->hide();
  this->yLabel->hide();
  this->zLabel->hide();
  this->viewPlaneLabel->hide();
  this->translateXCheckBox->hide();
  this->translateYCheckBox->hide();
  this->translateZCheckBox->hide();
  this->translateViewPlaneCheckBox->hide();
  this->rotateXCheckBox->hide();
  this->rotateYCheckBox->hide();
  this->rotateZCheckBox->hide();
  this->scaleXCheckBox->hide();
  this->scaleYCheckBox->hide();
  this->scaleZCheckBox->hide();
  this->scaleViewPlaneCheckBox->hide();
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
  d->translateXCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->translateYCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->translateZCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->translateViewPlaneCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->rotateVisibilityCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->rotateXCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->rotateYCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->rotateZCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->scaleVisibilityCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->scaleXCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->scaleYCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->scaleZCheckBox->setEnabled(d->DisplayNode != nullptr);
  d->interactionHandleScaleSlider->setEnabled(d->DisplayNode != nullptr);

  if (!d->DisplayNode)
    {
    return;
    }

  // Scale handles currently not implemented for all representations
  bool canDisplayScaleHandles = d->DisplayNode->GetCanDisplayScaleHandles();
  d->scaleVisibilityCheckBox->setEnabled(canDisplayScaleHandles);
  d->scaleXCheckBox->setEnabled(canDisplayScaleHandles);
  d->scaleYCheckBox->setEnabled(canDisplayScaleHandles);
  d->scaleZCheckBox->setEnabled(canDisplayScaleHandles);
  d->scaleViewPlaneCheckBox->setEnabled(canDisplayScaleHandles);
  d->scaleEnableLabel->setEnabled(canDisplayScaleHandles);

  bool wasBlocking = false;

  ////////
  // Interactive Mode
  wasBlocking = d->overallVisibilityCheckBox->blockSignals(true);
  d->overallVisibilityCheckBox->setChecked(d->DisplayNode->GetHandlesInteractive());
  d->overallVisibilityCheckBox->blockSignals(wasBlocking);

  //
  // Translation
  wasBlocking = d->translateVisibilityCheckBox->blockSignals(true);
  d->translateVisibilityCheckBox->setChecked(d->DisplayNode->GetTranslationHandleVisibility());
  d->translateVisibilityCheckBox->blockSignals(wasBlocking);

  bool* translationHandleAxes = d->DisplayNode->GetTranslationHandleComponentVisibility();
  wasBlocking = d->translateXCheckBox->blockSignals(true);
  d->translateXCheckBox->setChecked(translationHandleAxes[0]);
  d->translateXCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->translateYCheckBox->blockSignals(true);
  d->translateYCheckBox->setChecked(translationHandleAxes[1]);
  d->translateYCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->translateZCheckBox->blockSignals(true);
  d->translateZCheckBox->setChecked(translationHandleAxes[2]);
  d->translateZCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->translateViewPlaneCheckBox->blockSignals(true);
  d->translateViewPlaneCheckBox->setChecked(translationHandleAxes[3]);
  d->translateViewPlaneCheckBox->blockSignals(wasBlocking);

  //
  // Rotation
  wasBlocking = d->rotateVisibilityCheckBox->blockSignals(true);
  d->rotateVisibilityCheckBox->setChecked(d->DisplayNode->GetRotationHandleVisibility());
  d->rotateVisibilityCheckBox->blockSignals(wasBlocking);

  bool* rotationHandleAxes = d->DisplayNode->GetRotationHandleComponentVisibility();
  wasBlocking = d->rotateXCheckBox->blockSignals(true);
  d->rotateXCheckBox->setChecked(rotationHandleAxes[0]);
  d->rotateXCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->rotateYCheckBox->blockSignals(true);
  d->rotateYCheckBox->setChecked(rotationHandleAxes[1]);
  d->rotateYCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->rotateZCheckBox->blockSignals(true);
  d->rotateZCheckBox->setChecked(rotationHandleAxes[2]);
  d->rotateZCheckBox->blockSignals(wasBlocking);

  //
  // Scaling
  wasBlocking = d->scaleVisibilityCheckBox->blockSignals(true);
  d->scaleVisibilityCheckBox->setChecked(d->DisplayNode->GetScaleHandleVisibility() && canDisplayScaleHandles);
  d->scaleVisibilityCheckBox->blockSignals(wasBlocking);

  bool* scaleHandleAxes = d->DisplayNode->GetScaleHandleComponentVisibility();
  wasBlocking = d->scaleXCheckBox->blockSignals(true);
  d->scaleXCheckBox->setChecked(scaleHandleAxes[0] && canDisplayScaleHandles);
  d->scaleXCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->scaleYCheckBox->blockSignals(true);
  d->scaleYCheckBox->setChecked(scaleHandleAxes[1] && canDisplayScaleHandles);
  d->scaleYCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->scaleZCheckBox->blockSignals(true);
  d->scaleZCheckBox->setChecked(scaleHandleAxes[2] && canDisplayScaleHandles);
  d->scaleZCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->scaleViewPlaneCheckBox->blockSignals(true);
  d->scaleViewPlaneCheckBox->setChecked(scaleHandleAxes[3] && canDisplayScaleHandles);
  d->scaleViewPlaneCheckBox->blockSignals(wasBlocking);

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

  bool translationHandleAxes[4] = { d->translateXCheckBox->isChecked(),
                                    d->translateYCheckBox->isChecked(),
                                    d->translateZCheckBox->isChecked(),
                                    d->translateViewPlaneCheckBox->isChecked() };
  d->DisplayNode->SetTranslationHandleComponentVisibility(translationHandleAxes);
  d->DisplayNode->SetTranslationHandleVisibility(d->translateVisibilityCheckBox->isChecked());

  bool rotationHandleAxes[4] = { d->rotateXCheckBox->isChecked(),
                                 d->rotateYCheckBox->isChecked(),
                                 d->rotateZCheckBox->isChecked(), true };
  d->DisplayNode->SetRotationHandleComponentVisibility(rotationHandleAxes);
  d->DisplayNode->SetRotationHandleVisibility(d->rotateVisibilityCheckBox->isChecked());

  bool scaleHandleAxes[4] = { d->scaleXCheckBox->isChecked(),
                              d->scaleYCheckBox->isChecked(),
                              d->scaleZCheckBox->isChecked(),
                              d->scaleViewPlaneCheckBox->isChecked() };
  d->DisplayNode->SetScaleHandleComponentVisibility(scaleHandleAxes);
  d->DisplayNode->SetScaleHandleVisibility(d->scaleVisibilityCheckBox->isChecked());

  d->DisplayNode->SetInteractionHandleScale(d->interactionHandleScaleSlider->value());
}
