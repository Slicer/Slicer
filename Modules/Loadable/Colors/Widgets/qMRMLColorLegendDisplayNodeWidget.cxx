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

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkScalarBarWidget.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>

// CTK includes
#include <ctkVTKScalarBarWidget.h>
#include <ctkVTKTextPropertyWidget.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLDisplayableNode.h>

// Slicer Colors MRML includes
#include <vtkMRMLColorLegendDisplayNode.h>

// Qt includes
#include <QDebug>
#include <QSpinBox>

// Parameters Widgets includes
#include "qMRMLColorLegendDisplayNodeWidget.h"
#include "ui_qMRMLColorLegendDisplayNodeWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Colors
class qMRMLColorLegendDisplayNodeWidgetPrivate : public QWidget, public Ui_qMRMLColorLegendDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLColorLegendDisplayNodeWidget);
protected:
  qMRMLColorLegendDisplayNodeWidget* const q_ptr;
  typedef QWidget Superclass;

public:
  qMRMLColorLegendDisplayNodeWidgetPrivate(qMRMLColorLegendDisplayNodeWidget& object);
  virtual void setupUi(qMRMLColorLegendDisplayNodeWidget*);
  void init();

  /// color legend MRML node containing shown parameters
  vtkWeakPointer<vtkMRMLColorLegendDisplayNode> ColorLegendDisplayNode;
};

// --------------------------------------------------------------------------
qMRMLColorLegendDisplayNodeWidgetPrivate::qMRMLColorLegendDisplayNodeWidgetPrivate(
  qMRMLColorLegendDisplayNodeWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidgetPrivate::setupUi(qMRMLColorLegendDisplayNodeWidget* widget)
{
  this->Ui_qMRMLColorLegendDisplayNodeWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidgetPrivate::init()
{
  Q_Q(qMRMLColorLegendDisplayNodeWidget);

  // Set tooltip in label format widget
  this->LabelTextPropertyWidget->textEditWidget()->setToolTip(qMRMLColorLegendDisplayNodeWidget::tr(
    "<html><head><body>Format field uses printf function syntax. Example formats:<br>\
    - display with 1 fractional digits: <b>%.1f</b><br>\
    - display integer: <b>%.0f</b><br>\
    - display with 4 significant digits: <b>%.4g</b><br>\
    - string label annotation: <b>%s</b></body></html>"));

  // Radio buttons
  QObject::connect(this->ColorLegendOrientationButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
    q, SLOT(onColorLegendOrientationButtonClicked(QAbstractButton*)));
  QObject::connect(this->LabelTextButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
    q, SLOT(onLabelTextButtonClicked(QAbstractButton*)));

  // Position and size
  QObject::connect(this->PositionXSlider, SIGNAL(valueChanged(double)), q, SLOT(onPositionChanged()));
  QObject::connect(this->PositionYSlider, SIGNAL(valueChanged(double)), q, SLOT(onPositionChanged()));
  QObject::connect(this->ShortSideSizeSlider, SIGNAL(valueChanged(double)), q, SLOT(onSizeChanged()));
  QObject::connect(this->LongSideSizeSlider, SIGNAL(valueChanged(double)), q, SLOT(onSizeChanged()));

  // Title properties
  QObject::connect(this->TitleTextLineEdit, SIGNAL(textChanged(QString)),
                   q, SLOT(onTitleTextChanged(QString)));

  // Label properties
  QObject::connect(this->LabelTextPropertyWidget, SIGNAL(textChanged(QString)),
                   q, SLOT(onLabelFormatChanged(QString)));

  // QSpinBox
  QObject::connect( this->MaxNumberOfColorsSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(onMaximumNumberOfColorsChanged(int)));
  QObject::connect( this->NumberOfLabelsSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(onNumberOfLabelsChanged(int)));

  // QCheckBox
  QObject::connect( this->ColorLegendVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(onColorLegendVisibilityToggled(bool)));
}

//-----------------------------------------------------------------------------
// qSlicerColorLegendPropertiesWidget methods

//-----------------------------------------------------------------------------
qMRMLColorLegendDisplayNodeWidget::qMRMLColorLegendDisplayNodeWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qMRMLColorLegendDisplayNodeWidgetPrivate(*this) )
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLColorLegendDisplayNodeWidget::~qMRMLColorLegendDisplayNodeWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  this->Superclass::setMRMLScene(scene);
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::setMRMLColorLegendDisplayNode(vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);

  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ColorLegendDisplayNode, colorLegendDisplayNode, vtkCommand::ModifiedEvent,
    this, SLOT(updateWidgetFromMRML()));

  d->ColorLegendDisplayNode = colorLegendDisplayNode;
  d->DisplayNodeViewComboBox->setMRMLDisplayNode(d->ColorLegendDisplayNode);

  d->TitleTextPropertyWidget->setTextProperty(d->ColorLegendDisplayNode ? d->ColorLegendDisplayNode->GetTitleTextProperty() : nullptr);
  d->LabelTextPropertyWidget->setTextProperty(d->ColorLegendDisplayNode ? d->ColorLegendDisplayNode->GetLabelTextProperty() : nullptr);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode* qMRMLColorLegendDisplayNodeWidget::mrmlColorLegendDisplayNode()
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  return d->ColorLegendDisplayNode;
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::setMRMLColorLegendDisplayNode(vtkMRMLNode *node)
{
  vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = vtkMRMLColorLegendDisplayNode::SafeDownCast(node);
  this->setMRMLColorLegendDisplayNode(colorLegendDisplayNode);
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);

  this->setEnabled(d->ColorLegendDisplayNode != nullptr);
  if (!d->ColorLegendDisplayNode)
    {
    return;
    }

  // Set visibility checkbox
  QSignalBlocker blocker1(d->ColorLegendVisibilityCheckBox);
  d->ColorLegendVisibilityCheckBox->setChecked(d->ColorLegendDisplayNode->GetVisibility());

  // Setup color legend orientation
  QSignalBlocker blocker2(d->VerticalOrientationRadioButton);
  QSignalBlocker blocker3(d->HorizontalOrientationRadioButton);
  if (d->ColorLegendDisplayNode->GetOrientation() == vtkMRMLColorLegendDisplayNode::Vertical)
    {
    d->VerticalOrientationRadioButton->setChecked(true);
    }
  else // vtkMRMLColorLegendDisplayNode::Horizontal:
    {
    d->HorizontalOrientationRadioButton->setChecked(true);
    }

  QSignalBlocker blocker4(d->UseColorNameAsLabelTextRadioButton);
  QSignalBlocker blocker5(d->UseScalarValueAsLabelTextRadioButton);
  bool useColorNamesForLabels = d->ColorLegendDisplayNode->GetUseColorNamesForLabels();
  if (useColorNamesForLabels)
    {
    d->UseColorNameAsLabelTextRadioButton->setChecked(true);
    }
  else
    {
    d->UseScalarValueAsLabelTextRadioButton->setChecked(true);
    }
  // When using color names for labels then that determines
  // the number of colors and labels (each label is displayed)
  // therefore the MaxNumberOfColors and NumberOfLabels are ignored.
  // Indicate this to the user by disabling these options.
  d->MaxNumberOfColorsSpinBox->setEnabled(!useColorNamesForLabels);
  d->NumberOfLabelsSpinBox->setEnabled(!useColorNamesForLabels);

  QSignalBlocker blocker6(d->ShortSideSizeSlider);
  QSignalBlocker blocker7(d->LongSideSizeSlider);
  d->ShortSideSizeSlider->setValue(d->ColorLegendDisplayNode->GetSize()[0]);
  d->LongSideSizeSlider->setValue(d->ColorLegendDisplayNode->GetSize()[1]);

  QSignalBlocker blocker8(d->PositionXSlider);
  QSignalBlocker blocker9(d->PositionYSlider);
  d->PositionXSlider->setValue(d->ColorLegendDisplayNode->GetPosition()[0]);
  d->PositionYSlider->setValue(d->ColorLegendDisplayNode->GetPosition()[1]);

  // Title parameters
  std::string newTitle = d->ColorLegendDisplayNode->GetTitleText();
  QString currentTitle = d->TitleTextLineEdit->text();
  if (currentTitle.compare(QString::fromStdString(newTitle)))
    {
    QSignalBlocker blocker10(d->TitleTextLineEdit);
    d->TitleTextLineEdit->setText(newTitle.c_str());
    }

  // Label parameters
  std::string newFormat = d->ColorLegendDisplayNode->GetLabelFormat();
  QString currentFormat = d->LabelTextPropertyWidget->text();
  if (currentFormat.compare(QString::fromStdString(newFormat)))
    {
    QSignalBlocker blocker11(d->LabelTextPropertyWidget);
    d->LabelTextPropertyWidget->setText(newFormat.c_str());
    }

  // Number of colors and labels
  QSignalBlocker blocker12(d->MaxNumberOfColorsSpinBox);
  QSignalBlocker blocker13(d->NumberOfLabelsSpinBox);
  d->MaxNumberOfColorsSpinBox->setValue(d->ColorLegendDisplayNode->GetMaxNumberOfColors());
  d->NumberOfLabelsSpinBox->setValue(d->ColorLegendDisplayNode->GetNumberOfLabels());

  // Labels naming and arrangement
  QSignalBlocker blocker14(d->UseColorNameAsLabelTextRadioButton);
  QSignalBlocker blocker15(d->UseScalarValueAsLabelTextRadioButton);
  if (d->ColorLegendDisplayNode->GetUseColorNamesForLabels())
    {
    d->UseColorNameAsLabelTextRadioButton->setChecked(true);
    }
  else
    {
    d->UseScalarValueAsLabelTextRadioButton->setChecked(true);
    }
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onColorLegendVisibilityToggled(bool state)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);

  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }

  d->ColorLegendDisplayNode->SetVisibility(state);
}

//-----------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onColorLegendOrientationButtonClicked(QAbstractButton* button)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }

  if (button == d->HorizontalOrientationRadioButton)
    {
    d->ColorLegendDisplayNode->SetOrientation(vtkMRMLColorLegendDisplayNode::Horizontal);
    }
  else if (button == d->VerticalOrientationRadioButton)
    {
    d->ColorLegendDisplayNode->SetOrientation(vtkMRMLColorLegendDisplayNode::Vertical);
    }
}

//-----------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onLabelTextButtonClicked(QAbstractButton* button)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }

  MRMLNodeModifyBlocker blocker(d->ColorLegendDisplayNode);
  if (button == d->UseColorNameAsLabelTextRadioButton)
    {
    d->ColorLegendDisplayNode->SetUseColorNamesForLabels(true);
    d->ColorLegendDisplayNode->SetLabelFormat(d->ColorLegendDisplayNode->GetDefaultTextLabelFormat());
    }
  else if (button == d->UseScalarValueAsLabelTextRadioButton)
    {
    d->ColorLegendDisplayNode->SetUseColorNamesForLabels(false);
    d->ColorLegendDisplayNode->SetLabelFormat(d->ColorLegendDisplayNode->GetDefaultNumericLabelFormat());
    }
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onPositionChanged()
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }
  d->ColorLegendDisplayNode->SetPosition(d->PositionXSlider->value(), d->PositionYSlider->value());
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onSizeChanged()
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }
  d->ColorLegendDisplayNode->SetSize(d->ShortSideSizeSlider->value(), d->LongSideSizeSlider->value());
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onTitleTextChanged(const QString& titleText)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }
  d->ColorLegendDisplayNode->SetTitleText(titleText.toStdString());
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onLabelFormatChanged(const QString& labelFormat)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);
  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }
  d->ColorLegendDisplayNode->SetLabelFormat(labelFormat.toStdString());
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onMaximumNumberOfColorsChanged(int maxNumberOfColors)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);

  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }

  d->ColorLegendDisplayNode->SetMaxNumberOfColors(maxNumberOfColors);
}

//-----------------------------------------------------------------------------
void qMRMLColorLegendDisplayNodeWidget::onNumberOfLabelsChanged(int numberOfLabels)
{
  Q_D(qMRMLColorLegendDisplayNodeWidget);

  if (!d->ColorLegendDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << "failed: Invalid color legend display node";
    return;
    }

  d->ColorLegendDisplayNode->SetNumberOfLabels(numberOfLabels);
}
