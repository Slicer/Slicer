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

==============================================================================*/

// Qt includes
#include <QColor>

// CTK includes
#include <ctkUtils.h>

// qMRML includes
#include "qMRMLMarkupsDisplayNodeWidget.h"
#include "ui_qMRMLMarkupsDisplayNodeWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markupss
class qMRMLMarkupsDisplayNodeWidgetPrivate: public QWidget, public Ui_qMRMLMarkupsDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLMarkupsDisplayNodeWidget);

protected:
  qMRMLMarkupsDisplayNodeWidget* const q_ptr;
  typedef QWidget Superclass;

public:
  qMRMLMarkupsDisplayNodeWidgetPrivate(qMRMLMarkupsDisplayNodeWidget& object);
  void init();

  vtkWeakPointer<vtkMRMLMarkupsDisplayNode> MarkupsDisplayNode;
};

//------------------------------------------------------------------------------
qMRMLMarkupsDisplayNodeWidgetPrivate::qMRMLMarkupsDisplayNodeWidgetPrivate(qMRMLMarkupsDisplayNodeWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidgetPrivate::init()
{
  Q_Q(qMRMLMarkupsDisplayNodeWidget);
  this->setupUi(q);

  // use the ctk color dialog on the color picker buttons
  this->selectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);
  this->unselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // set up the display properties
  QObject::connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setVisibility(bool)));
  QObject::connect(this->selectedColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(onSelectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->unselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(onUnselectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->glyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
    q, SLOT(onGlyphTypeComboBoxChanged(QString)));
  QObject::connect(this->glyphSizeIsAbsoluteButton, SIGNAL(toggled(bool)),
    q, SLOT(setGlyphSizeIsAbsolute(bool)));
  QObject::connect(this->glyphScaleSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onGlyphScaleSliderWidgetChanged(double)));
  QObject::connect(this->glyphSizeSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onGlyphSizeSliderWidgetChanged(double)));
  QObject::connect(this->curveLineSizeIsAbsoluteButton, SIGNAL(toggled(bool)),
    q, SLOT(setCurveLineSizeIsAbsolute(bool)));
  QObject::connect(this->curveLineThicknessSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onCurveLineThicknessSliderWidgetChanged(double)));
  QObject::connect(this->curveLineDiameterSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onCurveLineDiameterSliderWidgetChanged(double)));
  QObject::connect(this->PointLabelsVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setPointLabelsVisibility(bool)));
  QObject::connect(this->textScaleSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onTextScaleSliderWidgetChanged(double)));

  QObject::connect(this->opacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onOpacitySliderWidgetChanged(double)));

  QObject::connect(this->interactionCheckBox, SIGNAL(stateChanged(int)),
    q, SLOT(onInteractionCheckBoxChanged(int)));

  QObject::connect(this->FillVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setFillVisibility(bool)));
  QObject::connect(this->OutlineVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setOutlineVisibility(bool)));
  QObject::connect(this->FillOpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onFillOpacitySliderWidgetChanged(double)));
  QObject::connect(this->OutlineOpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(onOutlineOpacitySliderWidgetChanged(double)));

  QObject::connect(this->OccludedVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setOccludedVisibility(bool)));
  QObject::connect(this->OccludedOpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setOccludedOpacity(double)));

  QObject::connect(this->OccludedVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setOccludedVisibility(bool)));

  this->TextFontFamilyComboBox->addItem(vtkTextProperty::GetFontFamilyAsString(VTK_ARIAL), VTK_ARIAL);
  this->TextFontFamilyComboBox->addItem(vtkTextProperty::GetFontFamilyAsString(VTK_COURIER), VTK_COURIER);
  this->TextFontFamilyComboBox->addItem(vtkTextProperty::GetFontFamilyAsString(VTK_TIMES), VTK_TIMES);

  QObject::connect(this->TextFontFamilyComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setTextProperty()));
  QObject::connect(this->TextBoldCheckBox,   SIGNAL(toggled(bool)), q, SLOT(setTextProperty()));
  QObject::connect(this->TextItalicCheckBox, SIGNAL(toggled(bool)), q, SLOT(setTextProperty()));
  QObject::connect(this->TextShadowCheckBox, SIGNAL(toggled(bool)), q, SLOT(setTextProperty()));
  QObject::connect(this->TextBackgroundColorPickerButton, SIGNAL(colorChanged(QColor)), q, SLOT(setTextProperty()));
  QObject::connect(this->TextBackgroundOpacitySlider, SIGNAL(valueChanged(double)), q, SLOT(setTextProperty()));

  // populate the glyph type combo box
  if (this->glyphTypeComboBox->count() == 0)
    {
    vtkNew<vtkMRMLMarkupsDisplayNode> displayNode;
    int min = displayNode->GetMinimumGlyphType();
    int max = displayNode->GetMaximumGlyphType();
    this->glyphTypeComboBox->setEnabled(false);
    for (int i = min; i <= max; i++)
      {
      this->glyphTypeComboBox->addItem(displayNode->GetGlyphTypeAsString(i), displayNode->GetGlyphTypeAsString(i));
      }
    this->glyphTypeComboBox->setEnabled(true);
    }
  // set the default value if not set
  if (this->glyphTypeComboBox->currentIndex() == 0)
    {
    vtkNew<vtkMRMLMarkupsDisplayNode> displayNode;
    QString glyphType = QString(displayNode->GetGlyphTypeAsString());
    this->glyphTypeComboBox->setEnabled(false);
    int index =  this->glyphTypeComboBox->findData(glyphType);
    if (index != -1)
      {
      this->glyphTypeComboBox->setCurrentIndex(index);
      }
    else
      {
      // glyph types start at 1, combo box is 0 indexed
      this->glyphTypeComboBox->setCurrentIndex(displayNode->GetGlyphType() - 1);
      }
    this->glyphTypeComboBox->setEnabled(true);
    }

  if (this->MarkupsDisplayNode.GetPointer())
    {
    q->setEnabled(true);
    q->setMRMLMarkupsDisplayNode(this->MarkupsDisplayNode);
    }

  this->glyphSizeSliderWidget->setVisible(this->glyphSizeIsAbsoluteButton->isChecked());
  this->glyphScaleSliderWidget->setHidden(this->glyphSizeIsAbsoluteButton->isChecked());

  this->curveLineDiameterSliderWidget->setVisible(this->glyphSizeIsAbsoluteButton->isChecked());
  this->curveLineThicknessSliderWidget->setHidden(this->glyphSizeIsAbsoluteButton->isChecked());

  // Disable until a valid display node is set
  this->setEnabled(false);
}

//------------------------------------------------------------------------------
qMRMLMarkupsDisplayNodeWidget::qMRMLMarkupsDisplayNodeWidget(QWidget *_parent)
  : QWidget(_parent)
  , d_ptr(new qMRMLMarkupsDisplayNodeWidgetPrivate(*this))
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLMarkupsDisplayNodeWidget::~qMRMLMarkupsDisplayNodeWidget()
{
  this->setMRMLMarkupsDisplayNode((vtkMRMLMarkupsDisplayNode*)nullptr);
}

//------------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* qMRMLMarkupsDisplayNodeWidget::mrmlMarkupsDisplayNode()const
{
  Q_D(const qMRMLMarkupsDisplayNodeWidget);
  return d->MarkupsDisplayNode;
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setMRMLMarkupsDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* node)
{
  this->setMRMLMarkupsDisplayNode(
    node ? vtkMRMLMarkupsDisplayNode::SafeDownCast(node->GetDisplayNode()) : nullptr);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setMRMLMarkupsNode(vtkMRMLNode* node)
{
  this->setMRMLMarkupsNode(vtkMRMLMarkupsNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setMRMLMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode* markupsDisplayNode)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (d->MarkupsDisplayNode == markupsDisplayNode)
    {
    return;
    }
  qvtkReconnect(d->MarkupsDisplayNode, markupsDisplayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->MarkupsDisplayNode = markupsDisplayNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  this->setEnabled(d->MarkupsDisplayNode != nullptr);
  d->DisplayNodeViewComboBox->setMRMLDisplayNode(d->MarkupsDisplayNode);
  d->pointFiducialProjectionWidget->setMRMLMarkupsDisplayNode(d->MarkupsDisplayNode);
  d->VisibilityCheckBox->setChecked(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetVisibility() : false);

  // update the display properties from the markups display node
  vtkSmartPointer<vtkMRMLMarkupsDisplayNode> markupsDisplayNode = d->MarkupsDisplayNode.GetPointer();
  if (!markupsDisplayNode)
    {
    // Create a temporary markups display node that we can retrieve defaults from.
    // We do not need the exact default display node properties, these are just placeholder
    // values shown in the disabled widget.
    markupsDisplayNode = vtkSmartPointer<vtkMRMLMarkupsDisplayNode>::New();
    }

  double* color = markupsDisplayNode->GetSelectedColor();
  d->selectedColorPickerButton->setColor(QColor::fromRgbF(color[0], color[1], color[2]));
  color = markupsDisplayNode->GetColor();
  d->unselectedColorPickerButton->setColor(QColor::fromRgbF(color[0], color[1], color[2]));
  d->opacitySliderWidget->setValue(markupsDisplayNode->GetOpacity());

  // glyph type
  QString glyphTypeStr = QString(markupsDisplayNode->GetGlyphTypeAsString());
  int glyphTypeIndex = d->glyphTypeComboBox->findData(glyphTypeStr);
  if (glyphTypeIndex>=0)
    {
    d->glyphTypeComboBox->setCurrentIndex(glyphTypeIndex);
    }

  d->glyphSizeIsAbsoluteButton->setChecked(!markupsDisplayNode->GetUseGlyphScale());

  // glyph scale
  double glyphScale = markupsDisplayNode->GetGlyphScale();
  // make sure that the slider can accommodate this scale
  if (glyphScale > d->glyphScaleSliderWidget->maximum())
    {
    d->glyphScaleSliderWidget->setMaximum(glyphScale);
    }
  d->glyphScaleSliderWidget->setValue(glyphScale);

  // glyph size
  double glyphSize = markupsDisplayNode->GetGlyphSize();
  // make sure that the slider can accommodate this scale
  if (glyphSize > d->glyphSizeSliderWidget->maximum())
    {
    d->glyphSizeSliderWidget->setMaximum(glyphSize);
    }
  d->glyphSizeSliderWidget->setValue(glyphSize);
  d->glyphSizeSliderWidget->setMRMLScene(markupsDisplayNode->GetScene());

  d->curveLineSizeIsAbsoluteButton->setChecked(markupsDisplayNode->GetCurveLineSizeMode() == vtkMRMLMarkupsDisplayNode::UseLineDiameter);

  // curve thickness
  double lineThicknessPercentage = markupsDisplayNode->GetLineThickness() * 100.0;
  // make sure that the slider can accommodate this scale
  if (lineThicknessPercentage > d->curveLineThicknessSliderWidget->maximum())
    {
    d->curveLineThicknessSliderWidget->setMaximum(lineThicknessPercentage);
    }
  d->curveLineThicknessSliderWidget->setValue(lineThicknessPercentage);

  // line diameter
  double lineDiameter = markupsDisplayNode->GetLineDiameter();
  // make sure that the slider can accommodate this scale
  if (lineDiameter > d->curveLineDiameterSliderWidget->maximum())
    {
    d->curveLineDiameterSliderWidget->setMaximum(lineDiameter);
    }
  d->curveLineDiameterSliderWidget->setValue(lineDiameter);

  // Only enable line size editing if not fiducial node
  bool lineSizeEnabled = (vtkMRMLMarkupsFiducialNode::SafeDownCast(markupsDisplayNode->GetDisplayableNode()) == nullptr);
  d->curveLineSizeIsAbsoluteButton->setEnabled(lineSizeEnabled);
  d->curveLineDiameterSliderWidget->setEnabled(lineSizeEnabled);
  d->curveLineThicknessSliderWidget->setEnabled(lineSizeEnabled);
  d->curveLineDiameterSliderWidget->setMRMLScene(markupsDisplayNode->GetScene());

  d->PointLabelsVisibilityCheckBox->setChecked(markupsDisplayNode->GetPointLabelsVisibility());

  // text scale
  double textScale = markupsDisplayNode->GetTextScale();
  // make sure that the slider can accommodate this scale
  if (textScale > d->textScaleSliderWidget->maximum())
    {
    d->textScaleSliderWidget->setMaximum(textScale);
    }
  d->textScaleSliderWidget->setValue(textScale);

  bool handlesInteractive = markupsDisplayNode->GetHandlesInteractive();
  d->interactionCheckBox->setChecked(handlesInteractive);

  bool wasBlocking = false;
  wasBlocking = d->FillVisibilityCheckBox->blockSignals(true);
  d->FillVisibilityCheckBox->setChecked(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetFillVisibility() : false);
  d->FillVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->OutlineVisibilityCheckBox->blockSignals(true);
  d->OutlineVisibilityCheckBox->setChecked(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetOutlineVisibility() : false);
  d->OutlineVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->FillOpacitySliderWidget->blockSignals(true);
  d->FillOpacitySliderWidget->setValue(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetFillOpacity() : 0.0);
  d->FillOpacitySliderWidget->blockSignals(wasBlocking);

  wasBlocking = d->OutlineOpacitySliderWidget->blockSignals(true);
  d->OutlineOpacitySliderWidget->setValue(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetOutlineOpacity() : 0.0);
  d->OutlineOpacitySliderWidget->blockSignals(wasBlocking);

  wasBlocking = d->OccludedVisibilityCheckBox->blockSignals(true);
  d->OccludedVisibilityCheckBox->setChecked(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetOccludedVisibility() : false);
  d->OccludedVisibilityCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->OccludedOpacitySliderWidget->blockSignals(true);
  d->OccludedOpacitySliderWidget->setValue(d->MarkupsDisplayNode ? d->MarkupsDisplayNode->GetOccludedOpacity() : 0.0);
  d->OccludedOpacitySliderWidget->blockSignals(wasBlocking);

  int fontFamily = VTK_ARIAL;
  bool textBold = false;
  bool textItalic = false;
  bool textShadow = false;
  double backgroundOpacity = 0.0;
  double textBackgroundColorF[3] = { 0.0, 0.0, 0.0 };

  if (markupsDisplayNode)
    {
    vtkTextProperty* property = markupsDisplayNode->GetTextProperty();
    fontFamily = property->GetFontFamily();
    textBold = property->GetBold();
    textItalic = property->GetItalic();
    textShadow = property->GetShadow();
    backgroundOpacity = property->GetBackgroundOpacity();
    property->GetBackgroundColor(textBackgroundColorF);
    }

  wasBlocking = d->TextFontFamilyComboBox->blockSignals(true);
  int fontFamilyIndex = 0;
  fontFamilyIndex = d->TextFontFamilyComboBox->findData(fontFamily);
  d->TextFontFamilyComboBox->setCurrentIndex(fontFamilyIndex);
  d->TextFontFamilyComboBox->blockSignals(wasBlocking);

  wasBlocking = d->TextBoldCheckBox->blockSignals(true);
  d->TextBoldCheckBox->setChecked(textBold);
  d->TextBoldCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->TextItalicCheckBox->blockSignals(true);
  d->TextItalicCheckBox->setChecked(textItalic);
  d->TextItalicCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->TextShadowCheckBox->blockSignals(true);
  d->TextShadowCheckBox->setChecked(textShadow);
  d->TextShadowCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->TextBackgroundOpacitySlider->blockSignals(true);
  d->TextBackgroundOpacitySlider->setValue(backgroundOpacity);
  d->TextBackgroundOpacitySlider->blockSignals(wasBlocking);

  wasBlocking = d->TextBackgroundColorPickerButton->blockSignals(true);
  d->TextBackgroundColorPickerButton->setColor(QColor::fromRgbF(textBackgroundColorF[0], textBackgroundColorF[1], textBackgroundColorF[2]));
  d->TextBackgroundColorPickerButton->blockSignals(wasBlocking);

  emit displayNodeChanged();
}

//------------------------------------------------------------------------------
vtkMRMLSelectionNode* qMRMLMarkupsDisplayNodeWidget::getSelectionNode(vtkMRMLScene *mrmlScene)
{
  vtkMRMLSelectionNode* selectionNode = nullptr;
  if (mrmlScene)
    {
    selectionNode =
      vtkMRMLSelectionNode::SafeDownCast(mrmlScene->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
    }
  return selectionNode;
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetVisibility(visible);
}

//------------------------------------------------------------------------------
bool qMRMLMarkupsDisplayNodeWidget::visibility()const
{
  Q_D(const qMRMLMarkupsDisplayNodeWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setPointLabelsVisibility(bool visible)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetPointLabelsVisibility(visible);
}

//------------------------------------------------------------------------------
bool qMRMLMarkupsDisplayNodeWidget::pointLabelsVisibility()const
{
  Q_D(const qMRMLMarkupsDisplayNodeWidget);
  return d->PointLabelsVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setGlyphSizeIsAbsolute(bool absolute)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetUseGlyphScale(!absolute);
}

//------------------------------------------------------------------------------
bool qMRMLMarkupsDisplayNodeWidget::glyphSizeIsAbsolute()const
{
  Q_D(const qMRMLMarkupsDisplayNodeWidget);
  return d->glyphSizeIsAbsoluteButton->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setCurveLineSizeIsAbsolute(bool absolute)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode.GetPointer())
    {
    return;
    }
  d->MarkupsDisplayNode->SetCurveLineSizeMode(absolute ?
    vtkMRMLMarkupsDisplayNode::UseLineDiameter : vtkMRMLMarkupsDisplayNode::UseLineThickness);
}

//------------------------------------------------------------------------------
bool qMRMLMarkupsDisplayNodeWidget::curveLineSizeIsAbsolute()const
{
  Q_D(const qMRMLMarkupsDisplayNodeWidget);
  return d->curveLineSizeIsAbsoluteButton->isChecked();
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onSelectedColorPickerButtonChanged(QColor color)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetSelectedColor(color.redF(), color.greenF(), color.blueF());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onUnselectedColorPickerButtonChanged(QColor color)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetColor(color.redF(), color.greenF(), color.blueF());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onGlyphTypeComboBoxChanged(QString value)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (value.isEmpty())
    {
    return;
    }
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetGlyphTypeFromString(value.toUtf8());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onGlyphScaleSliderWidgetChanged(double value)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetGlyphScale(value);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onGlyphSizeSliderWidgetChanged(double value)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetGlyphSize(value);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onCurveLineThicknessSliderWidgetChanged(double percentValue)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetLineThickness(percentValue * 0.01);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onCurveLineDiameterSliderWidgetChanged(double value)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetLineDiameter(value);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onTextScaleSliderWidgetChanged(double value)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetTextScale(value);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onOpacitySliderWidgetChanged(double value)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOpacity(value);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setMaximumMarkupsScale(double maxScale)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);

  if (maxScale > d->glyphScaleSliderWidget->maximum())
    {
    d->glyphScaleSliderWidget->setMaximum(maxScale);
    }
  if (maxScale > d->textScaleSliderWidget->maximum())
    {
    d->textScaleSliderWidget->setMaximum(maxScale);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setMaximumMarkupsSize(double maxSize)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);

  if (maxSize > d->glyphSizeSliderWidget->maximum())
    {
    d->glyphSizeSliderWidget->setMaximum(maxSize);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onInteractionCheckBoxChanged(int state)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetHandlesInteractive(state);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setFillVisibility(bool visibility)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetFillVisibility(visibility);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setOutlineVisibility(bool visibility)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOutlineVisibility(visibility);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onFillOpacitySliderWidgetChanged(double opacity)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetFillOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::onOutlineOpacitySliderWidgetChanged(double opacity)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOutlineOpacity(opacity);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setOccludedVisibility(bool visibility)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOccludedVisibility(visibility);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setOccludedOpacity(double OccludedOpacity)
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }
  d->MarkupsDisplayNode->SetOccludedOpacity(OccludedOpacity);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsDisplayNodeWidget::setTextProperty()
{
  Q_D(qMRMLMarkupsDisplayNodeWidget);
  if (!d->MarkupsDisplayNode)
    {
    return;
    }

  int fontFamily = d->TextFontFamilyComboBox->currentData().toInt();
  bool textBold = d->TextBoldCheckBox->isChecked();
  bool textItalic = d->TextItalicCheckBox->isChecked();
  bool textShadow = d->TextShadowCheckBox->isChecked();
  double backgroundOpacity = d->TextBackgroundOpacitySlider->value();
  QColor backgroundColor = d->TextBackgroundColorPickerButton->color();
  double backgroundColorF[3] = { backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF() };

  MRMLNodeModifyBlocker blocker(d->MarkupsDisplayNode);
  vtkSmartPointer<vtkTextProperty> textProperty = d->MarkupsDisplayNode->GetTextProperty();
  textProperty->SetFontFamily(fontFamily);
  textProperty->SetBold(textBold);
  textProperty->SetItalic(textItalic);
  textProperty->SetShadow(textShadow);
  textProperty->SetBackgroundOpacity(backgroundOpacity);
  textProperty->SetBackgroundColor(backgroundColorF);
}
