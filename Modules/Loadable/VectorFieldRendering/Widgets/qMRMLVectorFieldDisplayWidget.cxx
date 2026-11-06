/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Qt includes
#include <QColor>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QListView>
#include <QToolButton>

// STD includes
#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

// qMRML includes
#include "qMRMLVectorFieldDisplayWidget.h"
#include "qMRMLNodeComboBox.h"
#include "qMRMLScalarsDisplayWidget.h"
#include "ui_qMRMLVectorFieldDisplayWidget.h"

// CTK includes
#include <ctkCheckableComboBox.h>
#include <ctkDoubleSpinBox.h>

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorFieldPipeline.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
class qMRMLVectorFieldDisplayWidgetPrivate
  : public QWidget
  , public Ui_qMRMLVectorFieldDisplayWidget
{
  Q_DECLARE_PUBLIC(qMRMLVectorFieldDisplayWidget);

protected:
  qMRMLVectorFieldDisplayWidget* const q_ptr;
  typedef QWidget Superclass;

public:
  qMRMLVectorFieldDisplayWidgetPrivate(qMRMLVectorFieldDisplayWidget& object);
  void init();

  bool IsUpdatingWidgetFromMRML{ false };

  /// Display node that slots write to; nullptr while updating the widget from the node, so that values just read
  /// are not written back by the triggered slots.
  vtkMRMLVectorFieldDisplayNode* editedNode() const;

  /// The button that selects each visualization mode, keyed by the mode.
  std::map<int, QToolButton*> ModeToolButtons;

  /// The scale factor slider spans what suits the field, whatever the current factor is, so that a small saved factor
  /// does not leave a range that larger ones cannot be reached in. The range is fitted when another node or field is
  /// shown, grown for larger values, but not shrunk while dragging.
  bool ScaleFactorRangeFitted{ false };
  /// The mode the range was fitted in: glyphs and grids are scaled by factors of their own
  int ScaleFactorRangeMode{ -1 };
  void UpdateScaleFactorRange(vtkMRMLVectorFieldDisplayNode* displayNode);

  /// Form layout rows in layout order. Qt 5 keeps spacing for hidden rows, so unused rows are taken out of their
  /// layout and put back in place when shown again.
  struct FormRow
  {
    QWidget* Label{ nullptr };
    QWidget* Field{ nullptr };
  };
  std::vector<std::pair<QFormLayout*, std::vector<FormRow>>> FormRows;
  void RecordFormRows();
  void CompactFormRows();

  vtkWeakPointer<vtkMRMLVectorFieldDisplayNode> VectorFieldDisplayNode;
  vtkWeakPointer<vtkMRMLDisplayableNode> DisplayableNode;
  /// The filters of the display node, held while it is shown, so that the interpolation fit is computed only once
  /// even if no view shows the node
  vtkSmartPointer<vtkMRMLVectorFieldPipeline> FieldPipeline;
};

//------------------------------------------------------------------------------
namespace
{
/// Whether a widget is meant to be shown. Collapsed CTK sections hide children but store the intended state in the
/// "visibilityToParent" property; without that property, only an explicit hide counts.
bool IsShownToParent(QWidget* widget)
{
  QVariant visibilityToParent = widget->property("visibilityToParent");
  return (visibilityToParent.isValid() ? visibilityToParent.toBool() : !(widget->isHidden() && widget->testAttribute(Qt::WA_WState_ExplicitShowHide)));
}

/// Show or hide both widgets of a form row
void SetRowVisible(QWidget* label, QWidget* field, bool visible)
{
  label->setVisible(visible);
  field->setVisible(visible);
}

/// The display node that draws the surface of a model, wherever it is among the display
/// nodes of the model: the vector field display node can come before it.
vtkMRMLModelDisplayNode* GetSurfaceDisplayNode(vtkMRMLDisplayableNode* displayableNode)
{
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(displayableNode);
  for (int displayNodeIndex = 0; modelNode && displayNodeIndex < modelNode->GetNumberOfDisplayNodes(); ++displayNodeIndex)
  {
    vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetNthDisplayNode(displayNodeIndex));
    if (modelDisplayNode)
    {
      return modelDisplayNode;
    }
  }
  return nullptr;
}
} // namespace

//------------------------------------------------------------------------------
qMRMLVectorFieldDisplayWidgetPrivate::qMRMLVectorFieldDisplayWidgetPrivate(qMRMLVectorFieldDisplayWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidgetPrivate::init()
{
  Q_Q(qMRMLVectorFieldDisplayWidget);
  this->setupUi(q);

  this->GlyphTypeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Arrow"), vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow);
  this->GlyphTypeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Cone"), vtkMRMLVectorFieldDisplayNode::GlyphTypeCone);
  this->GlyphTypeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Box"), vtkMRMLVectorFieldDisplayNode::GlyphTypeBox);
  this->GlyphTypeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Cylinder"), vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder);
  this->GlyphTypeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Line"), vtkMRMLVectorFieldDisplayNode::GlyphTypeLine);
  this->GlyphTypeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Sphere"), vtkMRMLVectorFieldDisplayNode::GlyphTypeSphere);

  this->GlyphVectorScaleModeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Scale by magnitude"), false);
  this->GlyphVectorScaleModeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Scale by components"), true);

  this->ColorByComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Solid color"), 0);
  // Named for what the source offers when the widget is updated: the magnitude of the field,
  // or any array of a mesh
  this->ColorByComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Magnitude"), 1);

  this->SamplingRegionPartComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Points"), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints);
  this->SamplingRegionPartComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Surface"), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface);
  this->SamplingRegionPartComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Inside"), vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume);
  this->SamplingPatternComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Regular"), vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  this->SamplingPatternComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Random"), vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  this->MaskingModeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Show inside"), true);
  this->MaskingModeComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("Show outside"), false);

  // 0 lets the fit decide how far from the points the field is drawn
  this->InterpolationMaximumDistanceSpinBox->spinBox()->setSpecialValueText(qMRMLVectorFieldDisplayWidget::tr("Automatic"));

  // One checkable icon button per visualization mode, as the transform display had
  this->ModeToolButtons[vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph] = this->GlyphModeToolButton;
  this->ModeToolButtons[vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid] = this->GridModeToolButton;
  this->ModeToolButtons[vtkMRMLVectorFieldDisplayNode::VisualizationModeContour] = this->ContourModeToolButton;
  this->ModeToolButtons[vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline] = this->StreamlineModeToolButton;
  for (auto modeToButton : this->ModeToolButtons)
  {
    q->connect(modeToButton.second, SIGNAL(toggled(bool)), q, SLOT(setVisualizationModeFromButton(bool)));
  }

  q->connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setVisibility(bool)));
  q->connect(this->Visibility3DCheckBox, SIGNAL(toggled(bool)), q, SLOT(setVisibility3D(bool)));
  q->connect(this->Visibility2DCheckBox, SIGNAL(toggled(bool)), q, SLOT(setVisibility2D(bool)));
  q->connect(this->FieldArrayComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setFieldArray(int)));
  q->connect(this->InterpolateBetweenPointsCheckBox, SIGNAL(toggled(bool)), q, SLOT(setInterpolateBetweenPoints(bool)));
  q->connect(this->InterpolationMaximumDistanceSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setInterpolationMaximumDistance(double)));
  q->connect(this->ScaleFactorSliderWidget, SIGNAL(valueChanged(double)), q, SLOT(setScaleFactor(double)));
  q->connect(this->ScaleFactorReverseButton, SIGNAL(toggled(bool)), q, SLOT(setScaleFactorReversed(bool)));
  q->connect(this->FieldThresholdCheckBox, SIGNAL(toggled(bool)), q, SLOT(setThresholdEnabled(bool)));
  q->connect(this->FieldThresholdRangeWidget, SIGNAL(valuesChanged(double, double)), q, SLOT(setThresholdRange(double, double)));

  q->connect(this->SamplingRegionNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(setSamplingRegionNode(vtkMRMLNode*)));
  q->connect(this->SamplingRegionPartComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setSamplingRegionPart(int)));
  q->connect(this->SamplingPatternComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setSamplingPattern(int)));
  q->connect(this->SampleSpacingSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setSamplingSpacing(double)));
  q->connect(this->SamplingPointsNumberSpinBox, SIGNAL(valueChanged(int)), q, SLOT(setSamplingPointsNumber(int)));
  q->connect(this->MaskingRegionNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(setMaskingRegionNode(vtkMRMLNode*)));
  q->connect(this->MaskingModeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setMaskingMode(int)));
  q->connect(this->MaskingSegmentsComboBox, SIGNAL(checkedIndexesChanged()), q, SLOT(setMaskingSegments()));

  q->connect(this->GlyphTypeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setGlyphType(int)));
  q->connect(this->OrientationArrayComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setOrientationArray(int)));
  q->connect(this->ScaleArrayComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setScaleArray(int)));
  q->connect(this->GlyphVectorScaleModeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setVectorScaleMode(int)));
  q->connect(this->GlyphDiameterSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setGlyphDiameter(double)));
  q->connect(this->GlyphDiameterAbsoluteToolButton, SIGNAL(toggled(bool)), q, SLOT(setGlyphDiameterAbsolute(bool)));
  q->connect(this->GlyphShaftDiameterSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setGlyphShaftDiameter(double)));
  q->connect(this->GlyphTipLengthSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setGlyphTipLength(double)));
  q->connect(this->GlyphResolutionSpinBox, SIGNAL(valueChanged(int)), q, SLOT(setGlyphResolution(int)));
  q->connect(this->GlyphTipLength2DSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setGlyphTipLength2D(double)));
  q->connect(this->GlyphResolution2DSpinBox, SIGNAL(valueChanged(int)), q, SLOT(setGlyphResolution2D(int)));

  q->connect(this->GridSpacingSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setGridSpacing(double)));
  q->connect(this->GridShowNonWarpedCheckBox, SIGNAL(toggled(bool)), q, SLOT(setGridShowNonWarped(bool)));
  q->connect(this->GridNonWarpedColorPickerButton, SIGNAL(colorChanged(QColor)), q, SLOT(setGridNonWarpedColor(QColor)));
  q->connect(this->GridLinesVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setGridLinesVisibility(bool)));
  q->connect(this->GridFillCheckBox, SIGNAL(toggled(bool)), q, SLOT(setGridFill(bool)));
  q->connect(this->GridLineDiameterSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setGridLineDiameter(double)));
  q->connect(this->ContourLevelsLineEdit, SIGNAL(editingFinished()), q, SLOT(setContourLevels()));
  q->connect(this->StreamlineBidirectionalCheckBox, SIGNAL(toggled(bool)), q, SLOT(setStreamlineBidirectional(bool)));
  q->connect(this->StreamlineInitialStepSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setStreamlineInitialStep(double)));
  q->connect(this->MaximumPropagationSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setStreamlineMaximumPropagation(double)));
  q->connect(this->StreamlineTubeDiameterSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setStreamlineTubeDiameter(double)));
  q->connect(this->StreamlineSliceFadeDistanceSpinBox, SIGNAL(valueChanged(double)), q, SLOT(setStreamlineSliceFadeDistance(double)));

  q->connect(this->ColorByComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setColorBy(int)));
  q->connect(this->SameColoringAsSurfaceCheckBox, SIGNAL(toggled(bool)), q, SLOT(setSameColoringAsSurface(bool)));
  q->connect(this->ColorPickerButton, SIGNAL(colorChanged(QColor)), q, SLOT(setColor(QColor)));
  q->connect(this->OpacitySliderWidget, SIGNAL(valueChanged(double)), q, SLOT(setOpacity(double)));
  q->connect(this->SliceOpacitySliderWidget, SIGNAL(valueChanged(double)), q, SLOT(setSliceOpacity(double)));
  q->connect(this->SliceLineWidthSpinBox, SIGNAL(valueChanged(int)), q, SLOT(setSliceLineWidth(int)));

  this->RecordFormRows();

  q->setEnabled(false);
}

//------------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* qMRMLVectorFieldDisplayWidgetPrivate::editedNode() const
{
  return (this->IsUpdatingWidgetFromMRML ? nullptr : this->VectorFieldDisplayNode.GetPointer());
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidgetPrivate::RecordFormRows()
{
  Q_Q(qMRMLVectorFieldDisplayWidget);
  for (QFormLayout* formLayout : q->findChildren<QFormLayout*>())
  {
    // The coloring widget lays out its own rows
    if (this->ScalarsDisplayWidget->isAncestorOf(formLayout->parentWidget()))
    {
      continue;
    }
    std::vector<FormRow> rows;
    for (int row = 0; row < formLayout->rowCount(); ++row)
    {
      QLayoutItem* labelItem = formLayout->itemAt(row, QFormLayout::LabelRole);
      QLayoutItem* fieldItem = formLayout->itemAt(row, QFormLayout::FieldRole);
      if (!fieldItem)
      {
        fieldItem = formLayout->itemAt(row, QFormLayout::SpanningRole);
      }
      FormRow formRow;
      formRow.Label = (labelItem ? labelItem->widget() : nullptr);
      // A row whose field is a layout cannot be hidden, so it is never taken out
      formRow.Field = (fieldItem ? fieldItem->widget() : nullptr);
      rows.push_back(formRow);
    }
    this->FormRows.emplace_back(formLayout, rows);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidgetPrivate::CompactFormRows()
{
  for (auto& layoutRows : this->FormRows)
  {
    QFormLayout* formLayout = layoutRows.first;
    int rowIndex = 0; // where the next shown row goes
    for (const FormRow& formRow : layoutRows.second)
    {
      if (!formRow.Field)
      {
        // Always in the layout
        ++rowIndex;
        continue;
      }
      bool shown = (IsShownToParent(formRow.Field) || (formRow.Label && IsShownToParent(formRow.Label)));
      int currentRow = -1;
      QFormLayout::ItemRole role;
      formLayout->getWidgetPosition(formRow.Field, &currentRow, &role);
      bool inLayout = (currentRow >= 0);
      if (shown && !inLayout)
      {
        if (formRow.Label)
        {
          formLayout->insertRow(rowIndex, formRow.Label, formRow.Field);
        }
        else
        {
          formLayout->insertRow(rowIndex, formRow.Field);
        }
      }
      else if (!shown && inLayout)
      {
        // Taken out of the layout but kept: the widgets stay children of the widget, hidden
        QFormLayout::TakeRowResult taken = formLayout->takeRow(formRow.Field);
        delete taken.labelItem;
        delete taken.fieldItem;
      }
      if (shown)
      {
        ++rowIndex;
      }
    }
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidgetPrivate::UpdateScaleFactorRange(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  double value = std::abs(displayNode->GetEffectiveScaleFactor());
  // Grown when a larger value is set, so that the slider can show it
  if (this->ScaleFactorRangeFitted && value <= this->ScaleFactorSliderWidget->maximum())
  {
    return;
  }
  double defaultScaleFactor = displayNode->ComputeDefaultGlyphScaleFactor();
  // Without its data, the default of a model is not known yet: this is done again when the mesh arrives.
  // Other sources without a default, such as transforms, have vectors in mm.
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode());
  bool fieldKnown = (defaultScaleFactor > 0.0 || !modelNode || (modelNode->GetMesh() && modelNode->GetMesh()->GetNumberOfPoints() > 0));
  double maximum = std::max(10.0 * (defaultScaleFactor > 0.0 ? defaultScaleFactor : 1.0), value);
  // A factor of 1 is the true deformation of a grid
  if (displayNode->GetVisualizationMode() == vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid)
  {
    maximum = std::max(maximum, 1.0);
  }
  // Rounded up to a tidy 1, 2 or 5 times a power of ten
  double magnitude = std::pow(10.0, std::floor(std::log10(maximum)));
  double normalized = maximum / magnitude;
  const double tolerance = 1e-9;
  maximum = magnitude * (normalized <= 1.0 + tolerance ? 1.0 : normalized <= 2.0 + tolerance ? 2.0 : normalized <= 5.0 + tolerance ? 5.0 : 10.0);
  double step = maximum / 1000.0;
  int decimals = std::max(0, std::min(6, static_cast<int>(-std::floor(std::log10(step)))));
  this->ScaleFactorSliderWidget->setDecimals(decimals);
  this->ScaleFactorSliderWidget->setRange(0.0, maximum);
  this->ScaleFactorSliderWidget->setSingleStep(step);
  this->ScaleFactorRangeFitted = fieldKnown;
}

//------------------------------------------------------------------------------
qMRMLVectorFieldDisplayWidget::qMRMLVectorFieldDisplayWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLVectorFieldDisplayWidgetPrivate(*this))
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLVectorFieldDisplayWidget::~qMRMLVectorFieldDisplayWidget() = default;

//------------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* qMRMLVectorFieldDisplayWidget::mrmlVectorFieldDisplayNode() const
{
  Q_D(const qMRMLVectorFieldDisplayWidget);
  return d->VectorFieldDisplayNode;
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setMRMLVectorFieldDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLVectorFieldDisplayNode(vtkMRMLVectorFieldDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setMRMLVectorFieldDisplayNode(vtkMRMLVectorFieldDisplayNode* displayNode)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (d->VectorFieldDisplayNode == displayNode)
  {
    return;
  }

  qvtkReconnect(d->VectorFieldDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->VectorFieldDisplayNode = displayNode;
  d->FieldPipeline = vtkMRMLVectorFieldPipeline::GetSharedPipeline(displayNode);
  d->ScaleFactorRangeFitted = false;

  // Observe data changes of the field's displayable node to refresh the array lists; the events are source
  // specific (mesh, image, transform), so all are observed.
  vtkMRMLDisplayableNode* displayableNode = displayNode ? displayNode->GetDisplayableNode() : nullptr;
  qvtkReconnect(d->DisplayableNode, displayableNode, vtkMRMLModelNode::MeshModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->DisplayableNode, displayableNode, vtkMRMLVolumeNode::ImageDataModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->DisplayableNode, displayableNode, vtkMRMLTransformableNode::TransformModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->DisplayableNode = displayableNode;

  // The scalars widget edits the coloring properties (active scalar array, color
  // table, displayed range, threshold) of the same display node.
  d->ScalarsDisplayWidget->setMRMLDisplayNode(displayNode);

  this->updateWidgetFromMRML();
  emit mrmlVectorFieldDisplayNodeChanged(displayNode);
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->VectorFieldDisplayNode;
  if (!displayNode)
  {
    this->setEnabled(false);
    d->NoVectorArraysLabel->setVisible(false);
    return;
  }
  if (d->IsUpdatingWidgetFromMRML)
  {
    return;
  }
  // The slots that the widgets below trigger write nothing back while this is set
  d->IsUpdatingWidgetFromMRML = true;

  int glyphTypeIndex = d->GlyphTypeComboBox->findData(displayNode->GetGlyphType());
  d->GlyphTypeComboBox->setCurrentIndex(glyphTypeIndex >= 0 ? glyphTypeIndex : 0);

  // Array names come from the display node, not the sampled point set, so that the pipeline is not updated.
  std::vector<std::string> arrayNames;
  std::vector<int> arrayNumberOfComponents;
  displayNode->GetFieldArrayInfo(arrayNames, arrayNumberOfComponents);

  QString currentOrientationArrayName = QString::fromUtf8(displayNode->GetGlyphOrientationArrayName());
  QString currentScaleArrayName = QString::fromUtf8(displayNode->GetGlyphScaleArrayName());
  QString fieldArrayName = QString::fromUtf8(displayNode->GetFieldArrayName());

  d->FieldArrayComboBox->clear();
  d->FieldArrayComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("None"), QString());
  d->OrientationArrayComboBox->clear();
  // The array chosen above is what these follow unless the user picks another one here
  d->OrientationArrayComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("(Default)"), fieldArrayName);
  d->OrientationArrayComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("None"), QString());
  d->ScaleArrayComboBox->clear();
  d->ScaleArrayComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("(Default)"), fieldArrayName);
  d->ScaleArrayComboBox->addItem(qMRMLVectorFieldDisplayWidget::tr("None"), QString());

  int currentScaleArrayComponents = 0;
  for (size_t arrayIndex = 0; arrayIndex < arrayNames.size(); ++arrayIndex)
  {
    QString arrayName = QString::fromStdString(arrayNames[arrayIndex]);
    int numberOfComponents = arrayNumberOfComponents[arrayIndex];
    if (numberOfComponents == 3)
    {
      d->OrientationArrayComboBox->addItem(arrayName, arrayName);
      d->FieldArrayComboBox->addItem(arrayName, arrayName);
    }
    d->ScaleArrayComboBox->addItem(arrayName, arrayName);
    if (arrayName == currentScaleArrayName)
    {
      currentScaleArrayComponents = numberOfComponents;
    }
  }

  int fieldIndex = d->FieldArrayComboBox->findData(fieldArrayName);
  d->FieldArrayComboBox->setCurrentIndex(fieldIndex >= 0 ? fieldIndex : 0);
  // findData finds the "(Default)" entry first when the array is the one chosen above,
  // which is what shows that it is following it
  int orientationIndex = d->OrientationArrayComboBox->findData(currentOrientationArrayName);
  d->OrientationArrayComboBox->setCurrentIndex(orientationIndex >= 0 ? orientationIndex : 1);
  int scaleIndex = d->ScaleArrayComboBox->findData(currentScaleArrayName);
  d->ScaleArrayComboBox->setCurrentIndex(scaleIndex >= 0 ? scaleIndex : 1);

  // Sampled fields (vector volume, transform) have one fixed array, so there is nothing to choose.
  bool hasFixedArrays = displayNode->HasFixedFieldArrays();
  SetRowVisible(d->FieldArrayLabel, d->FieldArrayComboBox, !hasFixedArrays);
  // A model without a vector array draws nothing, whatever the settings are ("None" is always listed)
  bool noVectorArrays = (vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode()) != nullptr && d->FieldArrayComboBox->count() <= 1);
  d->NoVectorArraysLabel->setVisible(noVectorArrays);
  this->setEnabled(!noVectorArrays);

  int vectorScaleModeIndex = d->GlyphVectorScaleModeComboBox->findData(displayNode->GetGlyphScaleByComponents());
  d->GlyphVectorScaleModeComboBox->setCurrentIndex(vectorScaleModeIndex >= 0 ? vectorScaleModeIndex : 0);
  bool scaleArrayIsVector = (currentScaleArrayComponents == 3);
  d->GlyphVectorScaleModeLabel->setEnabled(scaleArrayIsVector);
  d->GlyphVectorScaleModeComboBox->setEnabled(scaleArrayIsVector);

  if (d->ScaleFactorRangeMode != displayNode->GetVisualizationMode())
  {
    d->ScaleFactorRangeMode = displayNode->GetVisualizationMode();
    d->ScaleFactorRangeFitted = false;
  }
  // The slider shows how much the vectors are scaled, the reverse button which way they point
  double scaleFactor = displayNode->GetEffectiveScaleFactor();
  d->UpdateScaleFactorRange(displayNode);
  d->ScaleFactorSliderWidget->setValue(std::abs(scaleFactor));
  d->ScaleFactorReverseButton->setChecked(scaleFactor < 0.0);

  // Interpolation between points: only where the source has points and no volumetric cells
  bool canInterpolate = displayNode->CanInterpolateBetweenPoints();
  bool interpolating = (canInterpolate && displayNode->GetInterpolateBetweenPoints());
  SetRowVisible(d->InterpolateBetweenPointsLabel, d->InterpolateBetweenPointsCheckBox, canInterpolate);
  d->InterpolateBetweenPointsCheckBox->setChecked(displayNode->GetInterpolateBetweenPoints());
  SetRowVisible(d->InterpolationMaximumDistanceLabel, d->InterpolationMaximumDistanceSpinBox, interpolating);
  d->InterpolationMaximumDistanceSpinBox->setValue(displayNode->GetInterpolationMaximumDistance());
  SetRowVisible(d->InterpolationResidualLabel, d->InterpolationResidualValueLabel, interpolating);
  if (interpolating)
  {
    // The fit approximates the data, so how closely it does is shown next to it
    double residual = (d->FieldPipeline ? d->FieldPipeline->GetInterpolationResidual() : -1.0);
    d->InterpolationResidualValueLabel->setText(residual >= 0.0 ? QString::number(residual, 'g', 3) : qMRMLVectorFieldDisplayWidget::tr("Not available"));
  }

  // Visualization mode, and the parameters that only apply to one of the modes
  int visualizationMode = displayNode->GetVisualizationMode();
  d->VisibilityCheckBox->setChecked(displayNode->GetVisibility() != 0);
  for (auto modeToButton : d->ModeToolButtons)
  {
    modeToButton.second->setVisible(displayNode->IsVisualizationModeSupported(modeToButton.first));
    modeToButton.second->setChecked(modeToButton.first == visualizationMode);
  }

  d->GridSpacingSpinBox->setValue(displayNode->GetGridSpacing());
  d->GridShowNonWarpedCheckBox->setChecked(displayNode->GetGridShowNonWarped());
  double* gridNonWarpedColor = displayNode->GetGridNonWarpedColor();
  d->GridNonWarpedColorPickerButton->setColor(QColor::fromRgbF(gridNonWarpedColor[0], gridNonWarpedColor[1], gridNonWarpedColor[2]));
  d->GridNonWarpedColorPickerButton->setEnabled(displayNode->GetGridShowNonWarped());
  d->GridFillCheckBox->setChecked(displayNode->GetGridFill());
  d->GridLinesVisibilityCheckBox->setChecked(displayNode->GetGridLinesVisibility());
  d->GridLineDiameterSpinBox->setValue(displayNode->GetGridLineDiameter());
  d->ContourLevelsLineEdit->setText(QString::fromStdString(displayNode->GetContourLevelsAsString()));
  // Without levels of its own a few are drawn over the color range: they are shown greyed, so
  // that it is clear what is drawn and that typing levels replaces them
  std::vector<double> effectiveLevels;
  displayNode->GetEffectiveContourLevels(effectiveLevels);
  QStringList effectiveLevelStrings;
  for (double level : effectiveLevels)
  {
    effectiveLevelStrings << QString::number(level, 'g', 3);
  }
  d->ContourLevelsLineEdit->setPlaceholderText(effectiveLevelStrings.join(" "));
  d->MaximumPropagationSpinBox->setValue(displayNode->GetStreamlineMaximumPropagation());
  d->StreamlineBidirectionalCheckBox->setChecked(displayNode->GetStreamlineBidirectional());
  d->StreamlineInitialStepSpinBox->setValue(displayNode->GetStreamlineInitialIntegrationStep());
  d->StreamlineTubeDiameterSpinBox->setValue(displayNode->GetStreamlineTubeDiameter());

  bool glyphMode = (visualizationMode == vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  bool gridMode = (visualizationMode == vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  bool contourMode = (visualizationMode == vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  bool streamlineMode = (visualizationMode == vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);

  SetRowVisible(d->GlyphTypeLabel, d->GlyphTypeComboBox, glyphMode);
  // Sources whose arrays are fixed (a sampled field always gives one vector array and its
  // magnitude) leave the user nothing to pick, so the array selectors are hidden.
  bool arrayChoice = (glyphMode && !hasFixedArrays);
  SetRowVisible(d->OrientationArrayLabel, d->OrientationArrayComboBox, arrayChoice);
  SetRowVisible(d->ScaleArrayLabel, d->ScaleArrayComboBox, arrayChoice);
  SetRowVisible(d->GlyphVectorScaleModeLabel, d->GlyphVectorScaleModeComboBox, arrayChoice);
  // The scale factor means something in every mode that scales the field, and it is named
  // the same in all of them.
  bool scaleFactorUsed = displayNode->IsScaleFactorUsed();
  SetRowVisible(d->ScaleFactorLabel, d->ScaleFactorSliderWidget, scaleFactorUsed);
  d->ScaleFactorReverseButton->setVisible(scaleFactorUsed);

  SetRowVisible(d->GridSpacingLabel, d->GridSpacingSpinBox, gridMode);
  SetRowVisible(d->GridShowNonWarpedLabel, d->GridShowNonWarpedWidget, gridMode);
  SetRowVisible(d->GridLinesVisibilityLabel, d->GridLinesVisibilityCheckBox, gridMode);
  SetRowVisible(d->GridFillLabel, d->GridFillCheckBox, gridMode);
  SetRowVisible(d->GridLineDiameterLabel, d->GridLineDiameterSpinBox, gridMode);
  SetRowVisible(d->ContourLevelsLabel, d->ContourLevelsLineEdit, contourMode);
  SetRowVisible(d->MaximumPropagationLabel, d->MaximumPropagationSpinBox, streamlineMode);
  SetRowVisible(d->StreamlineBidirectionalLabel, d->StreamlineBidirectionalCheckBox, streamlineMode);
  SetRowVisible(d->StreamlineInitialStepLabel, d->StreamlineInitialStepSpinBox, streamlineMode);
  SetRowVisible(d->StreamlineTubeDiameterLabel, d->StreamlineTubeDiameterSpinBox, streamlineMode);

  // Sampling region: mesh point fields are only sampled at their points (which can be thinned out);
  // isosurfaces of volumetric meshes use the cells, so nothing is sampled.
  bool samplingRegionUsed = displayNode->IsSamplingRegionUsed();
  bool isSampledSource = displayNode->CanSampleAtArbitraryPositions();
  SetRowVisible(d->SamplingRegionLabel, d->SamplingRegionNodeComboBox, isSampledSource && samplingRegionUsed);
  // Without a node a model or a volume is sampled itself, while a transform is defined
  // everywhere and is not shown in 3D views at all, so the empty choice says which
  if (vtkMRMLModelNode::SafeDownCast(d->DisplayableNode))
  {
    d->SamplingRegionNodeComboBox->setNoneDisplay(qMRMLVectorFieldDisplayWidget::tr("(This model)"));
  }
  else if (vtkMRMLVolumeNode::SafeDownCast(d->DisplayableNode))
  {
    d->SamplingRegionNodeComboBox->setNoneDisplay(qMRMLVectorFieldDisplayWidget::tr("(This volume)"));
  }
  else
  {
    // As transform display has always shown it: a region has to be selected
    d->SamplingRegionNodeComboBox->setNoneDisplay(qMRMLVectorFieldDisplayWidget::tr("Select a node"));
  }
  d->SamplingRegionNodeComboBox->setCurrentNode(displayNode->GetSamplingRegionNode());
  // Only the parts that the node has are offered, and the choice only when there is one
  int supportedParts = 0;
  QListView* partView = qobject_cast<QListView*>(d->SamplingRegionPartComboBox->view());
  for (int itemIndex = 0; itemIndex < d->SamplingRegionPartComboBox->count(); ++itemIndex)
  {
    bool supported = displayNode->IsSamplingRegionPartSupported(d->SamplingRegionPartComboBox->itemData(itemIndex).toInt());
    supportedParts += (supported ? 1 : 0);
    if (partView)
    {
      partView->setRowHidden(itemIndex, !supported);
    }
  }
  int partIndex = d->SamplingRegionPartComboBox->findData(displayNode->GetEffectiveSamplingRegionPart());
  d->SamplingRegionPartComboBox->setCurrentIndex(partIndex >= 0 ? partIndex : 0);
  bool partChoice = (samplingRegionUsed && supportedParts > 1);
  d->SamplingRegionPartComboBox->setVisible(partChoice);
  // Grids and isosurfaces are always built on a lattice
  bool usesSamples = (glyphMode || streamlineMode);
  SetRowVisible(d->SamplingLabel, d->SamplingWidget, partChoice || usesSamples);
  int patternIndex = d->SamplingPatternComboBox->findData(displayNode->GetSamplingPattern());
  d->SamplingPatternComboBox->setCurrentIndex(patternIndex >= 0 ? patternIndex : 0);
  d->SamplingPatternComboBox->setVisible(usesSamples);
  bool random = (displayNode->GetSamplingPattern() == vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  SetRowVisible(d->SamplingPointsNumberLabel, d->SamplingPointsNumberSpinBox, usesSamples && random);
  d->SamplingPointsNumberSpinBox->setValue(displayNode->GetSamplingPointsNumber());
  // One spacing for every mode: of the samples, of the seeds, or of the lattice that grids and
  // isosurfaces are built on, which only a field that can be sampled anywhere has
  bool samplingSpacingUsed = (displayNode->IsSamplingSpacingUsed() && (usesSamples || isSampledSource));
  SetRowVisible(d->SampleSpacingLabel, d->SampleSpacingSpinBox, samplingSpacingUsed);
  d->SampleSpacingSpinBox->setValue(displayNode->GetSamplingSpacing());

  // Where the field is shown
  vtkMRMLNode* maskingRegionNode = displayNode->GetMaskingRegionNode();
  d->MaskingRegionNodeComboBox->setCurrentNode(maskingRegionNode);
  int maskingModeIndex = d->MaskingModeComboBox->findData(displayNode->GetMaskingShowInside());
  d->MaskingModeComboBox->setCurrentIndex(maskingModeIndex >= 0 ? maskingModeIndex : 0);
  d->MaskingModeComboBox->setEnabled(maskingRegionNode != nullptr);
  vtkMRMLSegmentationNode* maskingSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(maskingRegionNode);
  SetRowVisible(d->MaskingSegmentsLabel, d->MaskingSegmentsComboBox, maskingSegmentationNode != nullptr);
  d->MaskingSegmentsComboBox->clear();
  std::vector<std::string> checkedSegmentIDs = displayNode->GetMaskingSegmentIDs();
  std::vector<std::string> segmentIDs;
  if (maskingSegmentationNode && maskingSegmentationNode->GetSegmentation())
  {
    maskingSegmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);
  }
  for (const std::string& segmentID : segmentIDs)
  {
    vtkSegment* segment = maskingSegmentationNode->GetSegmentation()->GetSegment(segmentID);
    QString name = QString::fromStdString(segment && segment->GetName() ? segment->GetName() : segmentID);
    d->MaskingSegmentsComboBox->addItem(name, QString::fromStdString(segmentID));
    bool checked = (std::find(checkedSegmentIDs.begin(), checkedSegmentIDs.end(), segmentID) != checkedSegmentIDs.end());
    d->MaskingSegmentsComboBox->setCheckState(d->MaskingSegmentsComboBox->model()->index(d->MaskingSegmentsComboBox->count() - 1, 0), checked ? Qt::Checked : Qt::Unchecked);
  }

  // Coloring: solid color, or active array through the color node (only vector magnitude for fixed-array sources).
  d->ColorByComboBox->setItemText(1, hasFixedArrays ? qMRMLVectorFieldDisplayWidget::tr("Magnitude") : qMRMLVectorFieldDisplayWidget::tr("Array"));
  bool colorByScalar = (displayNode->GetScalarVisibility() != 0);
  d->ColorByComboBox->setCurrentIndex(colorByScalar ? 1 : 0);
  // Following the coloring of the surface of the model, which is then edited there
  bool hasSurfaceDisplay = (GetSurfaceDisplayNode(d->DisplayableNode) != nullptr);
  bool followsSurface = (displayNode->GetColoringSourceDisplayNode() != nullptr);
  SetRowVisible(d->SameColoringAsSurfaceLabel, d->SameColoringAsSurfaceCheckBox, hasSurfaceDisplay || followsSurface);
  d->SameColoringAsSurfaceCheckBox->setChecked(followsSurface);
  // The scalars widget enables itself whenever it is updated, so the container is disabled
  for (QWidget* coloringWidget : { static_cast<QWidget*>(d->ColorByLabel), static_cast<QWidget*>(d->ColorByComboBox), static_cast<QWidget*>(d->ScalarsDisplayContainer) })
  {
    coloringWidget->setEnabled(!followsSurface);
  }
  SetRowVisible(d->ColorLabel, d->ColorPickerButton, !colorByScalar);
  d->ScalarsDisplayWidget->setVisible(colorByScalar);
  // The scalars widget has a visibility check box of its own, which this selector replaces,
  // and an array selector that means nothing for a source whose arrays are fixed.
  for (const char* hiddenChild : { "ScalarsVisibilityLabel", "ScalarsVisibilityCheckBox" })
  {
    QWidget* child = d->ScalarsDisplayWidget->findChild<QWidget*>(hiddenChild);
    if (child)
    {
      child->setVisible(false);
    }
  }
  // The label stays: the component of the fixed vectors can still be chosen next to it
  for (const char* arrayChild : { "ActiveScalarComboBox" })
  {
    QWidget* child = d->ScalarsDisplayWidget->findChild<QWidget*>(arrayChild);
    if (child)
    {
      child->setVisible(!hasFixedArrays);
    }
  }

  // Thresholding hides field parts with magnitude outside the range. Hide the whole row: a form layout keeps
  // the spacing of a row whose field is a layout, whatever is hidden in it.
  SetRowVisible(d->FieldThresholdLabel, d->FieldThresholdWidget, displayNode->IsThresholdUsed());
  bool thresholdEnabled = (displayNode->GetThresholdEnabled() != 0);
  d->FieldThresholdCheckBox->setChecked(thresholdEnabled);
  d->FieldThresholdRangeWidget->setEnabled(thresholdEnabled);
  double scalarRange[2] = { 0.0, 0.0 };
  displayNode->GetScalarRange(scalarRange);
  double thresholdRange[2] = { 0.0, 0.0 };
  displayNode->GetThresholdRange(thresholdRange);
  // Slider covers both scalar and threshold ranges, so a threshold outside the scalar range is not clamped.
  if (thresholdRange[1] >= thresholdRange[0])
  {
    double sliderRange[2] = { std::min(scalarRange[0], thresholdRange[0]), std::max(scalarRange[1], thresholdRange[1]) };
    if (sliderRange[1] > sliderRange[0])
    {
      d->FieldThresholdRangeWidget->setRange(sliderRange[0], sliderRange[1]);
    }
    d->FieldThresholdRangeWidget->setValues(thresholdRange[0], thresholdRange[1]);
  }
  else if (scalarRange[1] > scalarRange[0])
  {
    d->FieldThresholdRangeWidget->setRange(scalarRange[0], scalarRange[1]);
  }

  // The geometry of the glyph. Which of these mean anything depends on the glyph: only an
  // arrow has a shaft and a tip, only a round glyph has a resolution.
  int glyphType = displayNode->GetGlyphType();
  bool isArrow = (glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeArrow);
  bool isCone = (glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeCone);
  bool isSphere = (glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeSphere);
  bool hasDiameter = (isArrow || isCone || glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeBox || glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder);
  bool hasResolution = (isArrow || isCone || isSphere || glyphType == vtkMRMLVectorFieldDisplayNode::GlyphTypeCylinder);
  bool diameterAbsolute = displayNode->GetGlyphDiameterAbsolute();
  d->GlyphDiameterAbsoluteToolButton->setChecked(diameterAbsolute);
  d->GlyphDiameterSpinBox->setSuffix(diameterAbsolute ? qMRMLVectorFieldDisplayWidget::tr(" mm") : qMRMLVectorFieldDisplayWidget::tr(" %"));
  d->GlyphDiameterSpinBox->setValue(diameterAbsolute ? displayNode->GetGlyphDiameter() : displayNode->GetGlyphDiameterPercent());
  d->GlyphShaftDiameterSpinBox->setValue(displayNode->GetGlyphShaftDiameterPercent());
  d->GlyphTipLengthSpinBox->setValue(displayNode->GetGlyphTipLengthPercent());
  d->GlyphResolutionSpinBox->setValue(displayNode->GetGlyphResolution());
  d->GlyphTipLength2DSpinBox->setValue(displayNode->GetGlyphTipLengthPercent2D());
  d->GlyphResolution2DSpinBox->setValue(displayNode->GetGlyphResolution2D());
  // The toggle only says what unit the spin box next to it is in, so it goes with it, in one
  // container that hides the whole row
  SetRowVisible(d->GlyphDiameterLabel, d->GlyphDiameterWidget, glyphMode && hasDiameter);
  SetRowVisible(d->GlyphShaftDiameterLabel, d->GlyphShaftDiameterSpinBox, glyphMode && isArrow);
  SetRowVisible(d->GlyphTipLengthLabel, d->GlyphTipLengthSpinBox, glyphMode && isArrow);
  SetRowVisible(d->GlyphResolutionLabel, d->GlyphResolutionSpinBox, glyphMode && hasResolution);
  // Slice views draw a flat outline, which has a tip only for an arrow and a resolution
  // only for a round glyph.
  SetRowVisible(d->GlyphTipLength2DLabel, d->GlyphTipLength2DSpinBox, glyphMode && isArrow);
  SetRowVisible(d->GlyphResolution2DLabel, d->GlyphResolution2DSpinBox, glyphMode && isSphere);

  // 3D display
  d->Visibility3DCheckBox->setChecked(displayNode->GetVisibility3D() != 0);
  double* color = displayNode->GetColor();
  if (color)
  {
    d->ColorPickerButton->setColor(QColor::fromRgbF(color[0], color[1], color[2]));
  }
  d->OpacitySliderWidget->setValue(displayNode->GetOpacity());

  // Slice display
  d->Visibility2DCheckBox->setChecked(displayNode->GetVisibility2D() != 0);
  d->SliceLineWidthSpinBox->setValue(displayNode->GetSliceIntersectionThickness());
  d->SliceOpacitySliderWidget->setValue(displayNode->GetSliceIntersectionOpacity());
  SetRowVisible(d->StreamlineSliceFadeDistanceLabel, d->StreamlineSliceFadeDistanceSpinBox, streamlineMode);
  d->StreamlineSliceFadeDistanceSpinBox->setValue(displayNode->GetStreamlineSliceFadeDistance());

  // Rows that are hidden now leave no gap
  d->CompactFormRows();

  d->IsUpdatingWidgetFromMRML = false;
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::addColoringWidget(QWidget* widget)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (!widget)
  {
    return;
  }
  d->coloringLayout->addWidget(widget);
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setVisibility(bool visible)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetVisibility(visible);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setVisibility3D(bool visible)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetVisibility3D(visible);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setVisibility2D(bool visible)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetVisibility2D(visible);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setFieldArray(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    // A new field gets a scale factor of its own, which the slider is fitted to
    d->ScaleFactorRangeFitted = false;
    QString arrayName = d->FieldArrayComboBox->itemData(index).toString();
    MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->SetFieldArrayName(arrayName.isEmpty() ? nullptr : arrayName.toUtf8().constData());
    // If there's no color node set for a non empty array name, use a default
    if (!arrayName.isEmpty() && displayNode->GetColorNodeID() == nullptr)
    {
      displayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeFilePlasma.txt");
    }
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setInterpolateBetweenPoints(bool enabled)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetInterpolateBetweenPoints(enabled);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setInterpolationMaximumDistance(double distance)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetInterpolationMaximumDistance(distance);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setVisualizationModeFromButton(bool checked)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode();
  if (!displayNode || !checked)
  {
    return;
  }
  // The buttons are auto-exclusive, so exactly one of them is checked
  for (auto modeToButton : d->ModeToolButtons)
  {
    if (modeToButton.second == this->sender())
    {
      displayNode->SetVisualizationMode(modeToButton.first);
      return;
    }
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setScaleFactor(double value)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetEffectiveScaleFactor(d->ScaleFactorReverseButton->isChecked() ? -value : value);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setScaleFactorReversed(bool reversed)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    double magnitude = std::abs(displayNode->GetEffectiveScaleFactor());
    displayNode->SetEffectiveScaleFactor(reversed ? -magnitude : magnitude);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setThresholdEnabled(bool enabled)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetThresholdEnabled(enabled);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setThresholdRange(double minimum, double maximum)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetThresholdRange(minimum, maximum);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSamplingRegionNode(vtkMRMLNode* node)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetAndObserveSamplingRegionNode(node);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSamplingRegionPart(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode();
  if (displayNode && index >= 0)
  {
    displayNode->SetSamplingRegionPart(d->SamplingRegionPartComboBox->itemData(index).toInt());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSamplingPattern(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode();
  if (displayNode && index >= 0)
  {
    displayNode->SetSamplingPattern(d->SamplingPatternComboBox->itemData(index).toInt());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSamplingSpacing(double spacing)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetSamplingSpacing(spacing);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSamplingPointsNumber(int number)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetSamplingPointsNumber(number);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setMaskingRegionNode(vtkMRMLNode* node)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode();
  if (!displayNode)
  {
    return;
  }
  MRMLNodeModifyBlocker blocker(displayNode);
  // The segments that were chosen belong to the previous node
  if (node != displayNode->GetMaskingRegionNode())
  {
    displayNode->SetMaskingSegmentIDs(std::vector<std::string>());
  }
  displayNode->SetAndObserveMaskingRegionNode(node);
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setMaskingMode(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode();
  if (displayNode && index >= 0)
  {
    displayNode->SetMaskingShowInside(d->MaskingModeComboBox->itemData(index).toBool());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setMaskingSegments()
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode();
  if (!displayNode)
  {
    return;
  }
  std::vector<std::string> segmentIDs;
  for (const QModelIndex& checkedIndex : d->MaskingSegmentsComboBox->checkedIndexes())
  {
    segmentIDs.push_back(checkedIndex.data(Qt::UserRole).toString().toStdString());
  }
  displayNode->SetMaskingSegmentIDs(segmentIDs);
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphType(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphType(d->GlyphTypeComboBox->itemData(index).toInt());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setOrientationArray(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    QString arrayName = d->OrientationArrayComboBox->itemData(index).toString();
    displayNode->SetGlyphOrientationArrayName(arrayName.isEmpty() ? nullptr : arrayName.toUtf8().constData());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setScaleArray(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    QString arrayName = d->ScaleArrayComboBox->itemData(index).toString();
    displayNode->SetGlyphScaleArrayName(arrayName.isEmpty() ? nullptr : arrayName.toUtf8().constData());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setVectorScaleMode(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphScaleByComponents(d->GlyphVectorScaleModeComboBox->itemData(index).toBool());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphDiameter(double diameter)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode();
  if (!displayNode)
  {
    return;
  }
  if (displayNode->GetGlyphDiameterAbsolute())
  {
    displayNode->SetGlyphDiameter(diameter);
  }
  else
  {
    displayNode->SetGlyphDiameterPercent(diameter);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphDiameterAbsolute(bool absolute)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphDiameterAbsolute(absolute);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphShaftDiameter(double percent)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphShaftDiameterPercent(percent);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphTipLength(double percent)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphTipLengthPercent(percent);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphResolution(int resolution)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphResolution(resolution);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphTipLength2D(double percent)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphTipLengthPercent2D(percent);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGlyphResolution2D(int resolution)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGlyphResolution2D(resolution);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGridSpacing(double spacing)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGridSpacing(spacing);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGridShowNonWarped(bool show)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGridShowNonWarped(show);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGridNonWarpedColor(const QColor& color)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGridNonWarpedColor(color.redF(), color.greenF(), color.blueF());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGridLinesVisibility(bool visible)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGridLinesVisibility(visible);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGridFill(bool fill)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGridFill(fill);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setGridLineDiameter(double diameter)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetGridLineDiameter(diameter);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setContourLevels()
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetContourLevelsAsString(d->ContourLevelsLineEdit->text().toStdString());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setStreamlineBidirectional(bool bidirectional)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetStreamlineBidirectional(bidirectional);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setStreamlineInitialStep(double step)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetStreamlineInitialIntegrationStep(step);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setStreamlineMaximumPropagation(double length)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetStreamlineMaximumPropagation(length);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setStreamlineTubeDiameter(double diameter)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetStreamlineTubeDiameter(diameter);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setStreamlineSliceFadeDistance(double distance)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetStreamlineSliceFadeDistance(distance);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setColorBy(int index)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetScalarVisibility(d->ColorByComboBox->itemData(index).toInt() != 0);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSameColoringAsSurface(bool enabled)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetAndObserveColoringSourceDisplayNode(enabled ? GetSurfaceDisplayNode(d->DisplayableNode) : nullptr);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setColor(const QColor& color)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetColor(color.redF(), color.greenF(), color.blueF());
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setOpacity(double opacity)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetOpacity(opacity);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSliceOpacity(double opacity)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetSliceIntersectionOpacity(opacity);
  }
}

//------------------------------------------------------------------------------
void qMRMLVectorFieldDisplayWidget::setSliceLineWidth(int width)
{
  Q_D(qMRMLVectorFieldDisplayWidget);
  if (vtkMRMLVectorFieldDisplayNode* displayNode = d->editedNode())
  {
    displayNode->SetSliceIntersectionThickness(width);
  }
}
