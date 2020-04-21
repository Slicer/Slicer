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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>


// Slicer includes
#include "qMRMLUnitWidget.h"
#include "ui_qMRMLUnitWidget.h"

// MRML includes
#include "vtkMRMLUnitNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qMRMLUnitWidgetPrivate: public Ui_qMRMLUnitWidget
{
  Q_DECLARE_PUBLIC(qMRMLUnitWidget);
protected:
  qMRMLUnitWidget* const q_ptr;

public:
  qMRMLUnitWidgetPrivate(qMRMLUnitWidget& obj);

  void setupUi(qMRMLUnitWidget*);
  void clear();
  void updatePropertyWidgets();

  vtkMRMLUnitNode* CurrentUnitNode;
  qMRMLUnitWidget::UnitProperties DisplayFlags;
  qMRMLUnitWidget::UnitProperties EditableProperties;
};

//-----------------------------------------------------------------------------
// qMRMLUnitWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLUnitWidgetPrivate::qMRMLUnitWidgetPrivate(
  qMRMLUnitWidget& object)
  : q_ptr(&object)
{
  this->CurrentUnitNode = nullptr;
  this->DisplayFlags = qMRMLUnitWidget::All;
  this->EditableProperties = qMRMLUnitWidget::All;
  this->EditableProperties &= ~qMRMLUnitWidget::Quantity;
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidgetPrivate::setupUi(qMRMLUnitWidget* q)
{
  this->Ui_qMRMLUnitWidget::setupUi(q);

  QObject::connect(this->NameLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setName(QString)));
  QObject::connect(this->NameLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(nameChanged(QString)));
  QObject::connect(this->QuantityLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setQuantity(QString)));
  QObject::connect(this->QuantityLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(quantityChanged(QString)));

  QObject::connect(this->PrefixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setPrefix(QString)));
  QObject::connect(this->PrefixLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(prefixChanged(QString)));
  QObject::connect(this->SuffixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setSuffix(QString)));
  QObject::connect(this->SuffixLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(suffixChanged(QString)));

  QObject::connect(this->PrecisionSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setPrecision(int)));
  QObject::connect(this->PrecisionSpinBox, SIGNAL(valueChanged(int)),
    q, SIGNAL(precisionChanged(int)));

  QObject::connect(this->MinimumSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setMinimum(double)));
  QObject::connect(this->MinimumSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(minimumChanged(double)));
  QObject::connect(this->MaximumSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setMaximum(double)));
  QObject::connect(this->MaximumSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(maximumChanged(double)));

  QObject::connect(this->CoefficientSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setCoefficient(double)));
  QObject::connect(this->CoefficientSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(coefficientChanged(double)));
  QObject::connect(this->OffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setOffset(double)));
  QObject::connect(this->OffsetSpinBox, SIGNAL(valueChanged(double)),
    q, SIGNAL(offsetChanged(double)));

  QObject::connect(this->PresetNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(setUnitFromPreset(vtkMRMLNode*)));
  this->updatePropertyWidgets();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidgetPrivate::clear()
{
  this->NameLineEdit->clear();
  this->QuantityLineEdit->clear();
  this->PrefixLineEdit->clear();
  this->SuffixLineEdit->clear();
  this->PrecisionSpinBox->setValue(3);
  this->MinimumSpinBox->setValue(-1000);
  this->MaximumSpinBox->setValue(1000);
  this->CoefficientSpinBox->setValue(1.0);
  this->OffsetSpinBox->setValue(0.0);
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidgetPrivate::updatePropertyWidgets()
{
  this->PresetNodeComboBox->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Preset));
  this->PresetNodeComboBox->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Preset));
  this->PresetLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Preset));
  this->PresetLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Preset));

  this->SeparationLine->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Preset)
    && this->DisplayFlags > qMRMLUnitWidget::Preset);

  this->NameLineEdit->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Name));
  this->NameLineEdit->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Name));
  this->NameLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Name));
  this->NameLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Name));

  this->QuantityLineEdit->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Quantity));
  this->QuantityLineEdit->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Quantity));
  this->QuantityLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Quantity));
  this->QuantityLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Quantity));

  this->PrefixLineEdit->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Prefix));
  this->PrefixLineEdit->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Prefix));
  this->PrefixLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Prefix));
  this->PrefixLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Prefix));

  this->SuffixLineEdit->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Suffix));
  this->SuffixLineEdit->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Suffix));
  this->SuffixLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Suffix));
  this->SuffixLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Suffix));

  this->PrecisionSpinBox->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Precision));
  this->PrecisionSpinBox->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Precision));
  this->PrecisionLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Precision));
  this->PrecisionLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Precision));

  this->MinimumSpinBox->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Minimum));
  this->MinimumSpinBox->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Minimum));
  this->MinimumValueLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Minimum));
  this->MinimumValueLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Minimum));

  this->MaximumSpinBox->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Maximum));
  this->MaximumSpinBox->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Maximum));
  this->MaximumValueLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Maximum));
  this->MaximumValueLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Maximum));

  this->CoefficientSpinBox->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Coefficient));
  this->CoefficientSpinBox->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Coefficient));
  this->CoefficientLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Coefficient));
  this->CoefficientLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Coefficient));

   this->OffsetSpinBox->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Offset));
  this->OffsetSpinBox->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Offset));
  this->OffsetLabel->setVisible(
    this->DisplayFlags.testFlag(qMRMLUnitWidget::Offset));
  this->OffsetLabel->setEnabled(
    this->EditableProperties.testFlag(qMRMLUnitWidget::Offset));
}

//-----------------------------------------------------------------------------
// qMRMLUnitWidget methods

//-----------------------------------------------------------------------------
qMRMLUnitWidget::qMRMLUnitWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qMRMLUnitWidgetPrivate(*this) )
{
  Q_D(qMRMLUnitWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qMRMLUnitWidget::~qMRMLUnitWidget() = default;

//-----------------------------------------------------------------------------
qMRMLUnitWidget::UnitProperties qMRMLUnitWidget::displayedProperties() const
{
  Q_D(const qMRMLUnitWidget);
  return d->DisplayFlags;
}

//-----------------------------------------------------------------------------
qMRMLUnitWidget::UnitProperties qMRMLUnitWidget::editableProperties() const
{
  Q_D(const qMRMLUnitWidget);
  return d->EditableProperties;
}
//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  this->updateWidgetFromNode();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLUnitWidget::currentNode() const
{
  Q_D(const qMRMLUnitWidget);
  return d->CurrentUnitNode;
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setCurrentNode(vtkMRMLNode* node)
{
  Q_D(qMRMLUnitWidget);

  vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::SafeDownCast(node);
  this->qvtkReconnect(d->CurrentUnitNode, unitNode,
    vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromNode()));
  d->CurrentUnitNode = unitNode;

  this->updateWidgetFromNode();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::updateWidgetFromNode()
{
  Q_D(qMRMLUnitWidget);

  d->PresetNodeComboBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->NameLineEdit->setEnabled(d->CurrentUnitNode != nullptr);
  d->PrefixLineEdit->setEnabled(d->CurrentUnitNode != nullptr);
  d->SuffixLineEdit->setEnabled(d->CurrentUnitNode != nullptr);
  d->PrecisionSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->MinimumSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->MaximumSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->CoefficientSpinBox->setEnabled(d->CurrentUnitNode != nullptr);
  d->OffsetSpinBox->setEnabled(d->CurrentUnitNode != nullptr);

  if (!d->CurrentUnitNode)
    {
    d->clear();
    return;
    }

  // Preset
  bool modifying = d->PresetNodeComboBox->blockSignals(true);
  d->PresetNodeComboBox->addAttribute(
    "vtkMRMLUnitNode", "Quantity", d->CurrentUnitNode->GetQuantity());
  d->PresetNodeComboBox->setMRMLScene(this->mrmlScene());
  d->PresetNodeComboBox->setCurrentNode(nullptr);
  d->PresetNodeComboBox->blockSignals(modifying);

  d->NameLineEdit->setText(d->CurrentUnitNode->GetName());
  d->QuantityLineEdit->setText(d->CurrentUnitNode->GetQuantity());
  d->SuffixLineEdit->setText(d->CurrentUnitNode->GetSuffix());
  d->PrefixLineEdit->setText(QString(d->CurrentUnitNode->GetPrefix()));
  d->PrecisionSpinBox->setValue(d->CurrentUnitNode->GetPrecision());
  d->MinimumSpinBox->setValue(d->CurrentUnitNode->GetMinimumValue());
  d->MaximumSpinBox->setValue(d->CurrentUnitNode->GetMaximumValue());
  d->CoefficientSpinBox->setValue(d->CurrentUnitNode->GetDisplayCoefficient());
  d->OffsetSpinBox->setValue(d->CurrentUnitNode->GetDisplayOffset());
}

//-----------------------------------------------------------------------------
QString qMRMLUnitWidget::name() const
{
  Q_D(const qMRMLUnitWidget);
  return d->NameLineEdit->text();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setName(const QString& newName)
{
  Q_D(qMRMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetName(newName.toUtf8());
    }
}

//-----------------------------------------------------------------------------
QString qMRMLUnitWidget::quantity() const
{
  Q_D(const qMRMLUnitWidget);
  return d->QuantityLineEdit->text();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setQuantity(const QString& newQuantity)
{
  Q_D(qMRMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetQuantity(newQuantity.toUtf8());
    }
}

//-----------------------------------------------------------------------------
QString qMRMLUnitWidget::prefix() const
{
  Q_D(const qMRMLUnitWidget);
  return d->PrefixLineEdit->text();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setPrefix(const QString& newPrefix)
{
  Q_D(qMRMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetPrefix(newPrefix.toUtf8());
    }
  d->MaximumSpinBox->setPrefix(newPrefix);
  d->MinimumSpinBox->setPrefix(newPrefix);
}

//-----------------------------------------------------------------------------
QString qMRMLUnitWidget::suffix() const
{
  Q_D(const qMRMLUnitWidget);
  return d->SuffixLineEdit->text();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setSuffix(const QString& newSuffix)
{
  Q_D(qMRMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetSuffix(newSuffix.toUtf8());
    }
  d->MaximumSpinBox->setSuffix(newSuffix);
  d->MinimumSpinBox->setSuffix(newSuffix);
}

//-----------------------------------------------------------------------------
int qMRMLUnitWidget::precision() const
{
  Q_D(const qMRMLUnitWidget);
  return d->PrecisionSpinBox->value();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setPrecision(int newPrecision)
{
  Q_D(qMRMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetPrecision(newPrecision);
    }
  d->MaximumSpinBox->setDecimals(newPrecision);
  d->MinimumSpinBox->setDecimals(newPrecision);

  d->CoefficientSpinBox->setDecimals(newPrecision);
  d->OffsetSpinBox->setDecimals(newPrecision);
}

//-----------------------------------------------------------------------------
double qMRMLUnitWidget::minimum() const
{
  Q_D(const qMRMLUnitWidget);
  return d->MinimumSpinBox->value();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setMinimum(double newMin)
{
  Q_D(qMRMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetMinimumValue(newMin);
}

//-----------------------------------------------------------------------------
double qMRMLUnitWidget::maximum() const
{
  Q_D(const qMRMLUnitWidget);
  return d->MaximumSpinBox->value();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setMaximum(double newMax)
{
  Q_D(qMRMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetMaximumValue(newMax);
}

//-----------------------------------------------------------------------------
double qMRMLUnitWidget::coefficient() const
{
  Q_D(const qMRMLUnitWidget);
  return d->CoefficientSpinBox->value();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setCoefficient(double newCoeff)
{
  Q_D(qMRMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetDisplayCoefficient(newCoeff);
}

//-----------------------------------------------------------------------------
double qMRMLUnitWidget::offset() const
{
  Q_D(const qMRMLUnitWidget);
  return d->OffsetSpinBox->value();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setOffset(double newOffset)
{
  Q_D(qMRMLUnitWidget);

  if (!d->CurrentUnitNode)
    {
    return;
    }

  d->CurrentUnitNode->SetDisplayOffset(newOffset);
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setUnitFromPreset(vtkMRMLNode* presetNode)
{
  Q_D(qMRMLUnitWidget);

  vtkMRMLUnitNode *presetUnitNode = vtkMRMLUnitNode::SafeDownCast(presetNode);
  if (!presetUnitNode || !d->CurrentUnitNode)
    {
    return;
    }

  int disabledModify = d->CurrentUnitNode->StartModify();
  d->CurrentUnitNode->SetQuantity(presetUnitNode->GetQuantity());
  d->CurrentUnitNode->SetPrefix(presetUnitNode->GetPrefix());
  d->CurrentUnitNode->SetSuffix(presetUnitNode->GetSuffix());
  d->CurrentUnitNode->SetPrecision(presetUnitNode->GetPrecision());
  d->CurrentUnitNode->SetMinimumValue(presetUnitNode->GetMinimumValue());
  d->CurrentUnitNode->SetMaximumValue(presetUnitNode->GetMaximumValue());
  d->CurrentUnitNode->SetDisplayCoefficient(
    presetUnitNode->GetDisplayCoefficient());
  d->CurrentUnitNode->SetDisplayOffset(presetUnitNode->GetDisplayOffset());
  d->CurrentUnitNode->EndModify(disabledModify);
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget
::setDisplayedProperties(qMRMLUnitWidget::UnitProperties flag)
{
  Q_D(qMRMLUnitWidget);

  if (d->DisplayFlags == flag)
    {
    return;
    }

  d->DisplayFlags = flag;
  d->updatePropertyWidgets();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget
::setEditableProperties(qMRMLUnitWidget::UnitProperties properties)
{
  Q_D(qMRMLUnitWidget);

  if (d->EditableProperties == properties)
    {
    return;
    }

  d->EditableProperties = properties;
  d->updatePropertyWidgets();
}

