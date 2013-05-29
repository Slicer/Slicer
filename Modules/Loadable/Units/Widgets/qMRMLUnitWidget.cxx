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

// CTK includes
#include <ctkComboBox.h>

// Logic includes
#include "vtkSlicerUnitsLogic.h"

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qMRMLUnitWidget.h"
#include "ui_qMRMLUnitWidget.h"
#include "qMRMLWidget.h"

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
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

  virtual void setupUi(qMRMLUnitWidget*);
  void clear();

  vtkMRMLUnitNode* CurrentUnitNode;
};

//-----------------------------------------------------------------------------
// qMRMLUnitWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLUnitWidgetPrivate::qMRMLUnitWidgetPrivate(
  qMRMLUnitWidget& object)
  : q_ptr(&object)
{
  this->CurrentUnitNode = 0;
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidgetPrivate::setupUi(qMRMLUnitWidget* q)
{
  this->Ui_qMRMLUnitWidget::setupUi(q);

  QObject::connect(this->SuffixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setSuffix(QString)));
  QObject::connect(this->SuffixLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(suffixChanged(QString)));

  QObject::connect(this->PrefixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setPrefix(QString)));
  QObject::connect(this->PrefixLineEdit, SIGNAL(textChanged(QString)),
    q, SIGNAL(prefixChanged(QString)));

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

  QObject::connect(this->PresetNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(setUnitFromPreset(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidgetPrivate::clear()
{
  this->PrefixLineEdit->setText("");
  this->SuffixLineEdit->setText("");
  this->PrecisionSpinBox->setValue(3);
  this->MinimumSpinBox->setValue(-1000);
  this->MaximumSpinBox->setValue(1000);
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
qMRMLUnitWidget::~qMRMLUnitWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLUnitWidget);
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

  d->PrefixLineEdit->setEnabled(d->CurrentUnitNode != 0);
  d->SuffixLineEdit->setEnabled(d->CurrentUnitNode != 0);
  d->PrecisionSpinBox->setEnabled(d->CurrentUnitNode != 0);
  d->MinimumSpinBox->setEnabled(d->CurrentUnitNode != 0);
  d->MaximumSpinBox->setEnabled(d->CurrentUnitNode != 0);
  d->PresetNodeComboBox->setEnabled(d->CurrentUnitNode != 0);

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
  d->PresetNodeComboBox->setCurrentNode(0);
  d->PresetNodeComboBox->blockSignals(modifying);

  // Suffix
  d->SuffixLineEdit->setText(d->CurrentUnitNode->GetSuffix());

  // Prefix
  d->PrefixLineEdit->setText(QString(d->CurrentUnitNode->GetPrefix()));

  // Precision
  d->PrecisionSpinBox->setValue(d->CurrentUnitNode->GetPrecision());

  // Min
  d->MinimumSpinBox->setValue(d->CurrentUnitNode->GetMinimumValue());

  // Max
  d->MaximumSpinBox->setValue(d->CurrentUnitNode->GetMaximumValue());
}

//-----------------------------------------------------------------------------
QString qMRMLUnitWidget::prefix() const
{
  Q_D(const qMRMLUnitWidget);
  return d->PrefixLineEdit->text();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setPrefix(const QString& prefix)
{
  Q_D(qMRMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetPrefix(prefix.toLatin1());
    }
  d->MaximumSpinBox->setPrefix(prefix);
  d->MinimumSpinBox->setPrefix(prefix);
}

//-----------------------------------------------------------------------------
QString qMRMLUnitWidget::suffix() const
{
  Q_D(const qMRMLUnitWidget);
  return d->SuffixLineEdit->text();
}

//-----------------------------------------------------------------------------
void qMRMLUnitWidget::setSuffix(const QString& suffix)
{
  Q_D(qMRMLUnitWidget);

  if (d->CurrentUnitNode)
    {
    d->CurrentUnitNode->SetSuffix(suffix.toLatin1());
    }
  d->MaximumSpinBox->setSuffix(suffix);
  d->MinimumSpinBox->setSuffix(suffix);
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
  d->CurrentUnitNode->EndModify(disabledModify);
}
