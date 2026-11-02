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

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

  This file was further developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// qMRML includes
#include "qMRMLTransformInteractionWidget.h"
#include "ui_qMRMLTransformInteractionWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// STD includes
#include <initializer_list>

//------------------------------------------------------------------------------
class qMRMLTransformInteractionWidgetPrivate : public Ui_qMRMLTransformInteractionWidget
{
  Q_DECLARE_PUBLIC(qMRMLTransformInteractionWidget);

protected:
  qMRMLTransformInteractionWidget* const q_ptr;

public:
  qMRMLTransformInteractionWidgetPrivate(qMRMLTransformInteractionWidget& object);
  void init();

  /// Check boxes of one handle type (translation, rotation, scaling) in one view type: the handle itself and
  /// its components (X, Y, Z, view plane).
  struct HandleCheckBoxes
  {
    QCheckBox* Enabled{ nullptr };
    QCheckBox* Components[4]{ nullptr, nullptr, nullptr, nullptr };
  };
  HandleCheckBoxes Translation3D;
  HandleCheckBoxes Rotation3D;
  HandleCheckBoxes Scaling3D;
  HandleCheckBoxes TranslationSlice;
  HandleCheckBoxes RotationSlice;
  HandleCheckBoxes ScalingSlice;

  /// Show a value of the node without writing it back
  static void SetChecked(QCheckBox* checkBox, bool checked);
  static void SetValue(ctkSliderWidget* slider, double value);
  /// Show whether one kind of handle is offered and which of its components, both of which can
  /// only be edited while the handles are shown in that kind of view
  static void UpdateHandleCheckBoxes(const HandleCheckBoxes& checkBoxes, bool handleEnabled, const bool componentVisibility[4], bool viewEnabled);
  /// The component visibility that the check boxes of one kind of handle say
  static void GetComponentVisibility(const HandleCheckBoxes& checkBoxes, bool componentVisibility[4]);

  vtkWeakPointer<vtkMRMLTransformDisplayNode> TransformDisplayNode;
};

//------------------------------------------------------------------------------
qMRMLTransformInteractionWidgetPrivate::qMRMLTransformInteractionWidgetPrivate(qMRMLTransformInteractionWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidgetPrivate::init()
{
  Q_Q(qMRMLTransformInteractionWidget);
  this->setupUi(q);

  this->Translation3D = { this->InteractiveTranslation3DCheckBox,
                          { this->translateX3DCheckBox, this->translateY3DCheckBox, this->translateZ3DCheckBox, this->translateViewPlane3DCheckBox } };
  this->Rotation3D = { this->InteractiveRotation3DCheckBox, //
                       { this->rotateX3DCheckBox, this->rotateY3DCheckBox, this->rotateZ3DCheckBox, this->rotateViewPlane3DCheckBox } };
  this->Scaling3D = { this->InteractiveScaling3DCheckBox, //
                      { this->scaleX3DCheckBox, this->scaleY3DCheckBox, this->scaleZ3DCheckBox, this->scaleViewPlane3DCheckBox } };
  this->TranslationSlice = { this->InteractiveTranslationSliceCheckBox,
                             { this->translateXSliceCheckBox, this->translateYSliceCheckBox, this->translateZSliceCheckBox, this->translateViewPlaneSliceCheckBox } };
  this->RotationSlice = { this->InteractiveRotationSliceCheckBox,
                          { this->rotateXSliceCheckBox, this->rotateYSliceCheckBox, this->rotateZSliceCheckBox, this->rotateViewPlaneSliceCheckBox } };
  this->ScalingSlice = { this->InteractiveScalingSliceCheckBox, //
                         { this->scaleXSliceCheckBox, this->scaleYSliceCheckBox, this->scaleZSliceCheckBox, this->scaleViewPlaneSliceCheckBox } };

  QObject::connect(this->InteractionVisibleCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorVisibility(bool)));
  QObject::connect(this->InteractionVisible3dCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorVisibility3d(bool)));
  QObject::connect(this->InteractionVisible2dCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorVisibility2d(bool)));
  QObject::connect(this->Translation3D.Enabled, SIGNAL(toggled(bool)), q, SLOT(setEditorTranslationEnabled(bool)));
  QObject::connect(this->Rotation3D.Enabled, SIGNAL(toggled(bool)), q, SLOT(setEditorRotationEnabled(bool)));
  QObject::connect(this->Scaling3D.Enabled, SIGNAL(toggled(bool)), q, SLOT(setEditorScalingEnabled(bool)));
  QObject::connect(this->TranslationSlice.Enabled, SIGNAL(toggled(bool)), q, SLOT(setEditorTranslationSliceEnabled(bool)));
  QObject::connect(this->RotationSlice.Enabled, SIGNAL(toggled(bool)), q, SLOT(setEditorRotationSliceEnabled(bool)));
  QObject::connect(this->ScalingSlice.Enabled, SIGNAL(toggled(bool)), q, SLOT(setEditorScalingSliceEnabled(bool)));
  QObject::connect(this->EditorTranslationSliceAnywhereEnabledCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorTranslationSliceAnywhereEnabled(bool)));
  QObject::connect(this->editorTranslationSliceAnywhereSensitivitySlider, SIGNAL(valueChanged(double)), q, SLOT(setEditorTranslationSliceAnywhereSensitivity(double)));
  QObject::connect(this->interactionHandleScaleSlider, SIGNAL(valueChanged(double)), q, SLOT(setInteractionHandleScale(double)));
  QObject::connect(this->interactionHandleOpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(setInteractionHandleOpacity(double)));
  for (const HandleCheckBoxes* handle : { &this->Translation3D, &this->Rotation3D, &this->Scaling3D, &this->TranslationSlice, &this->RotationSlice, &this->ScalingSlice })
  {
    for (QCheckBox* componentCheckBox : handle->Components)
    {
      QObject::connect(componentCheckBox, SIGNAL(clicked()), q, SLOT(updateHandleComponentVisibility()));
    }
  }

  // The advanced handle options are shown by the button above them
  this->InteractiveAdvancedOptions3DFrame->hide();
  this->InteractiveAdvancedOptionsSliceFrame->hide();

  q->updateWidgetFromDisplayNode();
}

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidgetPrivate::SetChecked(QCheckBox* checkBox, bool checked)
{
  bool wasBlocking = checkBox->blockSignals(true);
  checkBox->setChecked(checked);
  checkBox->blockSignals(wasBlocking);
}

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidgetPrivate::SetValue(ctkSliderWidget* slider, double value)
{
  bool wasBlocking = slider->blockSignals(true);
  slider->setValue(value);
  slider->blockSignals(wasBlocking);
}

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidgetPrivate::UpdateHandleCheckBoxes(const HandleCheckBoxes& checkBoxes, bool handleEnabled, const bool componentVisibility[4], bool viewEnabled)
{
  SetChecked(checkBoxes.Enabled, handleEnabled);
  checkBoxes.Enabled->setEnabled(viewEnabled);
  for (int component = 0; component < 4; ++component)
  {
    SetChecked(checkBoxes.Components[component], componentVisibility[component]);
    checkBoxes.Components[component]->setEnabled(handleEnabled && viewEnabled);
  }
}

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidgetPrivate::GetComponentVisibility(const HandleCheckBoxes& checkBoxes, bool componentVisibility[4])
{
  for (int component = 0; component < 4; ++component)
  {
    componentVisibility[component] = checkBoxes.Components[component]->isChecked();
  }
}

//------------------------------------------------------------------------------
qMRMLTransformInteractionWidget::qMRMLTransformInteractionWidget(QWidget* newParent)
  : Superclass(newParent)
  , d_ptr(new qMRMLTransformInteractionWidgetPrivate(*this))
{
  Q_D(qMRMLTransformInteractionWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLTransformInteractionWidget::~qMRMLTransformInteractionWidget() = default;

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setMRMLTransformNode(vtkMRMLNode* node)
{
  this->setMRMLTransformNode(vtkMRMLTransformNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
{
  Q_D(qMRMLTransformInteractionWidget);
  vtkMRMLTransformDisplayNode* displayNode = (transformNode ? vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode()) : nullptr);
  if (d->TransformDisplayNode == displayNode)
  {
    return;
  }
  qvtkReconnect(d->TransformDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromDisplayNode()));
  d->TransformDisplayNode = displayNode;
  this->updateWidgetFromDisplayNode();
}

//------------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::updateWidgetFromDisplayNode()
{
  Q_D(qMRMLTransformInteractionWidget);
  vtkMRMLTransformDisplayNode* displayNode = d->TransformDisplayNode;
  this->setEnabled(displayNode != nullptr);
  if (!displayNode)
  {
    return;
  }
  d->SetChecked(d->InteractionVisibleCheckBox, displayNode->GetEditorVisibility());
  d->SetChecked(d->InteractionVisible3dCheckBox, displayNode->GetEditorVisibility3D());
  d->SetChecked(d->InteractionVisible2dCheckBox, displayNode->GetEditorSliceIntersectionVisibility());
  d->SetValue(d->interactionHandleScaleSlider, displayNode->GetInteractionScalePercent());
  d->SetValue(d->interactionHandleOpacitySlider, displayNode->GetInteractionHandleOpacity());

  // 3D views
  bool enabled3D = displayNode->GetEditorVisibility3D();
  bool componentVisibility[4] = { false, false, false, false };
  displayNode->GetTranslationHandleComponentVisibility3D(componentVisibility);
  d->UpdateHandleCheckBoxes(d->Translation3D, displayNode->GetEditorTranslationEnabled(), componentVisibility, enabled3D);
  displayNode->GetRotationHandleComponentVisibility3D(componentVisibility);
  d->UpdateHandleCheckBoxes(d->Rotation3D, displayNode->GetEditorRotationEnabled(), componentVisibility, enabled3D);
  displayNode->GetScaleHandleComponentVisibility3D(componentVisibility);
  d->UpdateHandleCheckBoxes(d->Scaling3D, displayNode->GetEditorScalingEnabled(), componentVisibility, enabled3D);

  // Slice views
  bool enabledSlice = displayNode->GetEditorSliceIntersectionVisibility();
  bool translationSliceEnabled = displayNode->GetEditorTranslationSliceEnabled();
  displayNode->GetTranslationHandleComponentVisibilitySlice(componentVisibility);
  d->UpdateHandleCheckBoxes(d->TranslationSlice, translationSliceEnabled, componentVisibility, enabledSlice);
  bool translationAnywhereEnabled = displayNode->GetEditorTranslationSliceAnywhereEnabled();
  d->SetChecked(d->EditorTranslationSliceAnywhereEnabledCheckBox, translationAnywhereEnabled);
  d->EditorTranslationSliceAnywhereEnabledCheckBox->setEnabled(translationSliceEnabled && enabledSlice);
  d->SetValue(d->editorTranslationSliceAnywhereSensitivitySlider, displayNode->GetEditorTranslationSliceAnywhereSensitivity());
  d->editorTranslationSliceAnywhereSensitivitySlider->setEnabled(translationAnywhereEnabled && translationSliceEnabled && enabledSlice);
  displayNode->GetRotationHandleComponentVisibilitySlice(componentVisibility);
  d->UpdateHandleCheckBoxes(d->RotationSlice, displayNode->GetEditorRotationSliceEnabled(), componentVisibility, enabledSlice);
  displayNode->GetScaleHandleComponentVisibilitySlice(componentVisibility);
  d->UpdateHandleCheckBoxes(d->ScalingSlice, displayNode->GetEditorScalingSliceEnabled(), componentVisibility, enabledSlice);
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorVisibility(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorVisibility(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorVisibility3d(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorVisibility3D(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorVisibility2d(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorSliceIntersectionVisibility(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorTranslationEnabled(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorTranslationEnabled(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorTranslationSliceEnabled(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorTranslationSliceEnabled(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorTranslationSliceAnywhereEnabled(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorTranslationSliceAnywhereEnabled(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorTranslationSliceAnywhereSensitivity(double sensitivity)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorTranslationSliceAnywhereSensitivity(sensitivity);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorRotationEnabled(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorRotationEnabled(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorRotationSliceEnabled(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorRotationSliceEnabled(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorScalingEnabled(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorScalingEnabled(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setEditorScalingSliceEnabled(bool enabled)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetEditorScalingSliceEnabled(enabled);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setInteractionHandleScale(double scalePercent)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetInteractionScalePercent(scalePercent);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::setInteractionHandleOpacity(double opacity)
{
  Q_D(qMRMLTransformInteractionWidget);
  if (d->TransformDisplayNode)
  {
    d->TransformDisplayNode->SetInteractionHandleOpacity(opacity);
  }
}

//-----------------------------------------------------------------------------
void qMRMLTransformInteractionWidget::updateHandleComponentVisibility()
{
  Q_D(qMRMLTransformInteractionWidget);
  vtkMRMLTransformDisplayNode* displayNode = d->TransformDisplayNode;
  if (!displayNode)
  {
    return;
  }
  // Only the kind of handle whose check box was clicked has changed; the others are set to
  // what they already are, which does not modify the node
  bool componentVisibility[4] = { false, false, false, false };
  d->GetComponentVisibility(d->Translation3D, componentVisibility);
  displayNode->SetTranslationHandleComponentVisibility3D(componentVisibility);
  d->GetComponentVisibility(d->Rotation3D, componentVisibility);
  displayNode->SetRotationHandleComponentVisibility3D(componentVisibility);
  d->GetComponentVisibility(d->Scaling3D, componentVisibility);
  displayNode->SetScaleHandleComponentVisibility3D(componentVisibility);
  d->GetComponentVisibility(d->TranslationSlice, componentVisibility);
  displayNode->SetTranslationHandleComponentVisibilitySlice(componentVisibility);
  d->GetComponentVisibility(d->RotationSlice, componentVisibility);
  displayNode->SetRotationHandleComponentVisibilitySlice(componentVisibility);
  d->GetComponentVisibility(d->ScalingSlice, componentVisibility);
  displayNode->SetScaleHandleComponentVisibilitySlice(componentVisibility);
}
