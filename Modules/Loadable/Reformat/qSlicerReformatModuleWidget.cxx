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

  This file was originally developed by Michael Jeulin-Lagarrigue, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// SlicerQt includes
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerTransformLogic.h"
#include "vtkSlicerReformatLogic.h"

#include "qSlicerReformatModuleWidget.h"
#include "ui_qSlicerReformatModule.h"

// MRML includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

//------------------------------------------------------------------------------
class qSlicerReformatModuleWidgetPrivate :
public Ui_qSlicerReformatModule
{
  Q_DECLARE_PUBLIC(qSlicerReformatModuleWidget);
protected:
  qSlicerReformatModuleWidget* const q_ptr;

public:
  qSlicerReformatModuleWidgetPrivate(
    qSlicerReformatModuleWidget& object);

  /// Update the widget interface
  void updateUi();

  /// Update the visibility controllers
  void updateVisibilityControllers();

  /// Update slice offset range and resolution (increment)
  void updateOffsetSlidersGroupBox();

  /// Update the origin position
  void updateOriginCoordinates();

  /// Update orientation selector state
  void updateOrientationGroupBox();

  /// Reset the slider
  void resetSlider(qMRMLLinearTransformSlider*);

  QButtonGroup* OriginCoordinateReferenceButtonGroup;
  vtkMRMLSliceNode* MRMLSliceNode;
  vtkMRMLSliceLogic* MRMLSliceLogic;
  double LastRotationValues[3]; // LR, PA, IS
};

//------------------------------------------------------------------------------
// qSlicerReformatModuleWidgetPrivate methods

//------------------------------------------------------------------------------
qSlicerReformatModuleWidgetPrivate::
qSlicerReformatModuleWidgetPrivate(
  qSlicerReformatModuleWidget& object)
  : q_ptr(&object)
{
  this->OriginCoordinateReferenceButtonGroup = 0;
  this->MRMLSliceNode = 0;
  this->MRMLSliceLogic = 0;
  this->LastRotationValues[0] = 0;
  this->LastRotationValues[1] = 0;
  this->LastRotationValues[2] = 0;
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidgetPrivate::updateUi()
{
  this->updateVisibilityControllers();
  this->updateOffsetSlidersGroupBox();
  this->updateOriginCoordinates();
  this->updateOrientationGroupBox();
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidgetPrivate::updateVisibilityControllers()
{
  // Check slice visibility
  bool wasVisibilityCheckBoxBlocking =
    this->VisibilityCheckBox->blockSignals(true);

  this->VisibilityCheckBox->setEnabled(this->MRMLSliceNode != 0);

  int visibility =
    (this->MRMLSliceNode) ? this->MRMLSliceNode->GetSliceVisible() : 0;
  this->VisibilityCheckBox->setChecked(visibility);

  this->VisibilityCheckBox->blockSignals(wasVisibilityCheckBoxBlocking);

  // Check reformat widget visibility
  bool wasVisibilityReformatWidgetCheckBoxBlocking =
    this->ReformatWidgetVisibilityCheckBox->blockSignals(true);

  this->ReformatWidgetVisibilityCheckBox->setEnabled(this->MRMLSliceNode != 0);
  int widgetVisibility =
    (this->MRMLSliceNode) ? this->MRMLSliceNode->GetWidgetVisible() : 0;
  this->ReformatWidgetVisibilityCheckBox->setChecked(widgetVisibility);

  this->ReformatWidgetVisibilityCheckBox->blockSignals(
    wasVisibilityReformatWidgetCheckBoxBlocking);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidgetPrivate::updateOffsetSlidersGroupBox()
{
  if (!this->MRMLSliceNode || !this->MRMLSliceLogic)
    {
    return;
    }

  bool wasBlocking = this->OffsetSlider->blockSignals(true);

  // Set the scale increments to match the z spacing (rotated into slice space)
  const double * sliceSpacing =
    this->MRMLSliceLogic->GetLowestVolumeSliceSpacing();
  Q_ASSERT(sliceSpacing);
  double offsetResolution = sliceSpacing ? sliceSpacing[2] : 0;
  this->OffsetSlider->setSingleStep(offsetResolution);
  this->OffsetSlider->setPageStep(offsetResolution);

  // Set slice offset range to match the field of view
  // Calculate the number of slices in the current range
  double sliceBounds[6] = {0, -1, 0, -1, 0, -1};
  this->MRMLSliceLogic->GetLowestVolumeSliceBounds(sliceBounds);
  Q_ASSERT(sliceBounds[4] <= sliceBounds[5]);
  this->OffsetSlider->setRange(sliceBounds[4], sliceBounds[5]);

  // Update slider position
  this->OffsetSlider->setValue(this->MRMLSliceLogic->GetSliceOffset());
  this->OffsetSlider->blockSignals(wasBlocking);
}

void qSlicerReformatModuleWidgetPrivate::updateOriginCoordinates()
{
  Q_Q(qSlicerReformatModuleWidget);

  vtkSlicerReformatLogic* transformLogic =
    vtkSlicerReformatLogic::SafeDownCast(q->logic());

  if (!this->MRMLSliceNode || !transformLogic)
    {
    return;
    }

  // Block signals
  //bool wasOnPlaneXBlocking = this->OnPlaneXdoubleSpinBox->blockSignals(true);
  //bool wasOnPlaneYBlocking = this->OnPlaneYdoubleSpinBox->blockSignals(true);
  bool wasInVolumeXBlocking = this->InVolumeXdoubleSpinBox->blockSignals(true);
  bool wasInVolumeYBlocking = this->InVolumeYdoubleSpinBox->blockSignals(true);
  bool wasInVolumeZBlocking = this->InVolumeZdoubleSpinBox->blockSignals(true);

  // Update volumes extremums
  double volumeBounds[6] = {0, 0, 0, 0, 0, 0};
  transformLogic->GetVolumeBounds(this->MRMLSliceNode, volumeBounds);

  this->InVolumeXdoubleSpinBox->setMinimum(volumeBounds[0]);
  this->InVolumeXdoubleSpinBox->setMaximum(volumeBounds[1]);
  this->InVolumeYdoubleSpinBox->setMinimum(volumeBounds[2]);
  this->InVolumeYdoubleSpinBox->setMaximum(volumeBounds[3]);
  this->InVolumeZdoubleSpinBox->setMinimum(volumeBounds[4]);
  this->InVolumeZdoubleSpinBox->setMaximum(volumeBounds[5]);

  // TODO : Update plane extremums
  /*double sliceBounds[6] = {0, 0, 0, 0, 0, 0};
  this->MRMLSliceLogic->GetLowestVolumeSliceBounds(sliceBounds);

  this->OnPlaneXdoubleSpinBox->setMinimum(sliceBounds[0]);
  this->OnPlaneXdoubleSpinBox->setMaximum(sliceBounds[1]);
  this->OnPlaneYdoubleSpinBox->setMinimum(sliceBounds[2]);
  this->OnPlaneYdoubleSpinBox->setMaximum(sliceBounds[3]);*/

  // Update volumes origin coordinates
  vtkMatrix4x4* sliceToRAS = this->MRMLSliceNode->GetSliceToRAS();
  this->InVolumeXdoubleSpinBox->setValue(sliceToRAS->GetElement(0,3));
  this->InVolumeYdoubleSpinBox->setValue(sliceToRAS->GetElement(1,3));
  this->InVolumeZdoubleSpinBox->setValue(sliceToRAS->GetElement(2,3));

  // TODO : Update plane origin coordinates

  // Reset signals blocking
  //this->OnPlaneXdoubleSpinBox->blockSignals(wasOnPlaneXBlocking);
  //this->OnPlaneYdoubleSpinBox->blockSignals(wasOnPlaneYBlocking);
  this->InVolumeXdoubleSpinBox->blockSignals(wasInVolumeXBlocking);
  this->InVolumeYdoubleSpinBox->blockSignals(wasInVolumeYBlocking);
  this->InVolumeZdoubleSpinBox->blockSignals(wasInVolumeZBlocking);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidgetPrivate::updateOrientationGroupBox()
{
  if (!this->MRMLSliceNode)
    {
    this->SliceOrientationSelector->setCurrentIndex(-1);
    return;
    }

  // Update the selector
  int index = this->SliceOrientationSelector->findText(
      QString::fromStdString(this->MRMLSliceNode->GetOrientationString()));
  Q_ASSERT(index>=0 && index <=4);
  this->SliceOrientationSelector->setCurrentIndex(index);

  // Update the normal spinboxes
  bool wasNormalXBlocking = this->NormalXdoubleSpinBox->blockSignals(true);
  bool wasNormalYBlocking = this->NormalYdoubleSpinBox->blockSignals(true);
  bool wasNormalZBlocking = this->NormalZdoubleSpinBox->blockSignals(true);

  vtkMatrix4x4* sliceToRAS = this->MRMLSliceNode->GetSliceToRAS();
  this->NormalXdoubleSpinBox->setValue(sliceToRAS->GetElement(0,2));
  this->NormalYdoubleSpinBox->setValue(sliceToRAS->GetElement(1,2));
  this->NormalZdoubleSpinBox->setValue(sliceToRAS->GetElement(2,2));

  this->NormalXdoubleSpinBox->blockSignals(wasNormalXBlocking);
  this->NormalYdoubleSpinBox->blockSignals(wasNormalYBlocking);
  this->NormalZdoubleSpinBox->blockSignals(wasNormalZBlocking);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidgetPrivate::
resetSlider(qMRMLLinearTransformSlider* slider)
{
  bool wasSliderBlocking = slider->blockSignals(true);
  slider->reset();

  if (slider == this->LRSlider)
    {
    this->LastRotationValues[0] = slider->value();
    }
  else if (slider == this->PASlider)
    {
    this->LastRotationValues[1] = slider->value();
    }
  else if (slider == this->ISSlider)
    {
    this->LastRotationValues[2] = slider->value();
    }

  slider->blockSignals(wasSliderBlocking);
}

//------------------------------------------------------------------------------
// qSlicerReformatModuleWidget methods

//------------------------------------------------------------------------------
qSlicerReformatModuleWidget::qSlicerReformatModuleWidget(
  QWidget* _parent) : Superclass( _parent ),
  d_ptr( new qSlicerReformatModuleWidgetPrivate(*this) )
{
}

//------------------------------------------------------------------------------
qSlicerReformatModuleWidget::~qSlicerReformatModuleWidget()
{
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::setup()
{
  Q_D(qSlicerReformatModuleWidget);
  d->setupUi(this);

  // Connect node selector with module itself
  this->connect(d->VisibilityCheckBox,
                   SIGNAL(toggled(bool)),
                   this, SLOT(onSliceVisibilityChanged(bool)));
  this->connect(d->ReformatWidgetVisibilityCheckBox,
                   SIGNAL(toggled(bool)),
                   this, SLOT(onReformatWidgetVisibilityChanged(bool)));
  this->connect(d->SliceNodeSelector,
                SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onNodeSelected(vtkMRMLNode*)));

  // Connect Slice offset slider
  this->connect(d->OffsetSlider, SIGNAL(valueChanged(double)),
                this, SLOT(setSliceOffsetValue(double)), Qt::QueuedConnection);
  this->connect(d->OffsetSlider, SIGNAL(valueIsChanging(double)),
                this, SLOT(onTrackSliceOffsetValueChanged(double)),
                Qt::QueuedConnection);

  // Add origin coordinate reference button to a button group
  d->OriginCoordinateReferenceButtonGroup =
    new QButtonGroup(d->OriginCoordinateReferenceButtonGroup);
  d->OriginCoordinateReferenceButtonGroup->addButton(d->OnPlaneRadioButton,
    qSlicerReformatModuleWidget::ONPLANE);
  d->OriginCoordinateReferenceButtonGroup->addButton(d->InVolumeRadioButton,
    qSlicerReformatModuleWidget::INVOLUME);

  // Plane coordinate system is not supported for now
  d->CoordinateReferenceGroupBox->setHidden(true);
  d->InVolumeRadioButton->setChecked(true);
  d->OnPlaneGroupBox->setHidden(true);

  // Connect button group
  this->connect(d->OriginCoordinateReferenceButtonGroup,
                SIGNAL(buttonPressed(int)),
                SLOT(onOriginCoordinateReferenceButtonPressed(int)));

  // Connect World Coordinates of origin spinBoxes
  this->connect(d->InVolumeXdoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onWorldPositionChanged()));
  this->connect(d->InVolumeYdoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onWorldPositionChanged()));
  this->connect(d->InVolumeZdoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onWorldPositionChanged()));

  // Connect Orientation selector
  this->connect(d->SliceOrientationSelector, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(onSliceOrientationChanged(QString)));

  // Connect the recenter
  this->connect(d->CenterPushButton, SIGNAL(pressed()),
                this, SLOT(centerSliceNode()));

  // Connect slice normal spinBoxes
  this->connect(d->NormalXdoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onSliceNormalChanged()));
  this->connect(d->NormalYdoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onSliceNormalChanged()));
  this->connect(d->NormalZdoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onSliceNormalChanged()));

  // Connect Slice rotation sliders
  this->connect(d->LRSlider, SIGNAL(valueChanged(double)),
                this, SLOT(onSliderRotationChanged(double)));
  this->connect(d->PASlider, SIGNAL(valueChanged(double)),
                this, SLOT(onSliderRotationChanged(double)));
  this->connect(d->ISSlider, SIGNAL(valueChanged(double)),
                this, SLOT(onSliderRotationChanged(double)));
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::
onMRMLSliceNodeModified(vtkObject* caller)
{
  Q_D(qSlicerReformatModuleWidget);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);
  if (!sliceNode)
    {
    return;
    }

  d->updateUi();
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::onNodeSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerReformatModuleWidget);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(node);

  // Listen for SliceNode changes
  this->qvtkReconnect(d->MRMLSliceNode, sliceNode,
    vtkCommand::ModifiedEvent,
    this, SLOT(onMRMLSliceNodeModified(vtkObject*)));

  d->MRMLSliceNode = sliceNode;
  d->MRMLSliceLogic =
    this->logic()->GetMRMLApplicationLogic()->GetSliceLogic(d->MRMLSliceNode);

  d->updateUi();
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::onSliceVisibilityChanged(bool visible)
{
  std::cout << "onSliceVisibilityChanged" << std::endl;
  Q_D(qSlicerReformatModuleWidget);
  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetSliceVisible(visible);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::
onReformatWidgetVisibilityChanged(bool visible)
{
  Q_D(qSlicerReformatModuleWidget);
  if (!d->MRMLSliceNode)
    {
    return;
    }

  if (visible)
    {
    d->MRMLSliceNode->SetSliceVisible(visible);
    }

  d->MRMLSliceNode->SetWidgetVisible(visible);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::
onOriginCoordinateReferenceButtonPressed(int ref)
{
  Q_D(qSlicerReformatModuleWidget);

  if (ref == qSlicerReformatModuleWidget::INVOLUME)
    {
    d->OnPlaneGroupBox->setHidden(true);
    d->InVolumeGroupBox->setHidden(false);
    }
  else
    {
    d->OnPlaneGroupBox->setHidden(false);
    d->InVolumeGroupBox->setHidden(true);
    }
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::
setSliceOffsetValue(double offset)
{
  Q_D(qSlicerReformatModuleWidget);
  if (!d->MRMLSliceLogic)
    {
    return;
    }

  d->MRMLSliceLogic->StartSliceOffsetInteraction();
  d->MRMLSliceLogic->SetSliceOffset(offset);
  d->MRMLSliceLogic->EndSliceOffsetInteraction();
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::
onTrackSliceOffsetValueChanged(double offset)
{
  Q_D(qSlicerReformatModuleWidget);
  if (!d->MRMLSliceLogic)
    {
    return;
    }

  d->MRMLSliceLogic->StartSliceOffsetInteraction();
  d->MRMLSliceLogic->SetSliceOffset(offset);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::onWorldPositionChanged()
{
  Q_D(qSlicerReformatModuleWidget);

  vtkSlicerReformatLogic* transformLogic =
    vtkSlicerReformatLogic::SafeDownCast(this->logic());

  if (!d->MRMLSliceNode || !transformLogic)
    {
    return;
    }

  double worldCoordinates[3];
  worldCoordinates[0] = d->InVolumeXdoubleSpinBox->value();
  worldCoordinates[1] = d->InVolumeYdoubleSpinBox->value();
  worldCoordinates[2] = d->InVolumeZdoubleSpinBox->value();

  // Insert the widget translation
  transformLogic->SetSliceOrigin(d->MRMLSliceNode, worldCoordinates);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::onSliceNormalChanged()
{
  Q_D(qSlicerReformatModuleWidget);

  vtkSlicerReformatLogic* transformLogic =
    vtkSlicerReformatLogic::SafeDownCast(this->logic());

  if (!d->MRMLSliceNode || !transformLogic)
    {
    return;
    }

  double sliceNormal[3];
  sliceNormal[0] = d->NormalXdoubleSpinBox->value();
  sliceNormal[1] = d->NormalYdoubleSpinBox->value();
  sliceNormal[2] = d->NormalZdoubleSpinBox->value();

  // Insert the widget translation
  transformLogic->SetSliceNormal(d->MRMLSliceNode, sliceNormal);
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::
onSliceOrientationChanged(const QString& orientation)
{
  Q_D(qSlicerReformatModuleWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  // Reset the Rotation Sliders
  d->resetSlider(d->LRSlider);
  d->resetSlider(d->PASlider);
  d->resetSlider(d->ISSlider);

#ifndef QT_NO_DEBUG
  QStringList expectedOrientation;
  expectedOrientation << tr("Axial") << tr("Sagittal")
                      << tr("Coronal") << tr("Reformat");
  Q_ASSERT(expectedOrientation.contains(orientation));
#endif

  d->MRMLSliceNode->SetOrientation(orientation.toLatin1());
  d->MRMLSliceNode->SetOrientationString(orientation.toLatin1());
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::
onSliderRotationChanged(double rotation)
{
  Q_D(qSlicerReformatModuleWidget);

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->SetMatrix(d->MRMLSliceNode->GetSliceToRAS());

  if (this->sender() == d->LRSlider)
    {
    // Reset PA & IS sliders
    d->resetSlider(d->PASlider);
    d->resetSlider(d->ISSlider);

    // Rotate on LR given the angle with the last value reccorded
    transform->RotateX(rotation-d->LastRotationValues[0]);

    // Update last value and apply the transform
    d->LastRotationValues[0] = rotation;
    }
  else if (this->sender() == d->PASlider)
    {
    // Reset LR & IS sliders
    d->resetSlider(d->LRSlider);
    d->resetSlider(d->ISSlider);

    // Rotate on PA given the angle with the last value reccorded
    transform->RotateY(rotation-d->LastRotationValues[1]);

    // Update last value and apply the transform
    d->LastRotationValues[1] = rotation;
    }
  else if (this->sender() == d->ISSlider)
    {
      // Reset LR & PA sliders
      d->resetSlider(d->LRSlider);
      d->resetSlider(d->PASlider);

      // Rotate on PA given the angle with the last value reccorded
      transform->RotateZ(rotation-d->LastRotationValues[2]);

      // Update last value and apply the transform
      d->LastRotationValues[2] = rotation;
    }

  // Apply the transform
  d->MRMLSliceNode->GetSliceToRAS()->DeepCopy(transform->GetMatrix());
  d->MRMLSliceNode->UpdateMatrices();
}

//------------------------------------------------------------------------------
void qSlicerReformatModuleWidget::centerSliceNode()
{
  Q_D(qSlicerReformatModuleWidget);

  vtkSlicerReformatLogic* transformLogic =
    vtkSlicerReformatLogic::SafeDownCast(this->logic());

  if (!d->MRMLSliceNode || !d->MRMLSliceLogic || !transformLogic)
    {
    return;
    }

  // TODO add the recenter given the Plane Referentiel

  // Retrieve the center given the volume bounds
  double bounds[6], center[3];
  transformLogic->GetVolumeBounds(d->MRMLSliceNode, bounds);
  vtkSlicerReformatLogic::GetCenterFromBounds(bounds, center);

  // Apply the center
  transformLogic->SetSliceOrigin(d->MRMLSliceNode, center);
}
