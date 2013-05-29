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

// Qt includes
#include <QDebug>
#include <QWidget>

// SlicerQt includes
#include "qSlicerUnitsModuleWidget.h"
#include "ui_qSlicerUnitsModuleWidget.h"

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLUnitNode.h"

// STD
#include <vector>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerUnitsModuleWidgetPrivate: public Ui_qSlicerUnitsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerUnitsModuleWidget);
protected:
  qSlicerUnitsModuleWidget* const q_ptr;

public:
  qSlicerUnitsModuleWidgetPrivate(qSlicerUnitsModuleWidget& obj);

  virtual void setupUi(qSlicerUnitsModuleWidget*);
};

//-----------------------------------------------------------------------------
// qSlicerUnitsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerUnitsModuleWidgetPrivate::qSlicerUnitsModuleWidgetPrivate(
  qSlicerUnitsModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidgetPrivate::setupUi(qSlicerUnitsModuleWidget* q)
{
  this->Ui_qSlicerUnitsModuleWidget::setupUi(q);

  QObject::connect(this->UnitNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(updateWidgetFromNode()));

  QObject::connect(this->QuantityComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(updateNodeQuantityFromWidget()));
  QObject::connect(this->SuffixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setSuffix(QString)));
  QObject::connect(this->PrefixLineEdit, SIGNAL(textChanged(QString)),
    q, SLOT(setPrefix(QString)));
  QObject::connect(this->PrecisionSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setPrecision(int)));
  QObject::connect(this->MaximumSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setMinimum(double)));
  QObject::connect(this->MinimumSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setMaximum(double)));

  // \todo remove this for supporting user-custom nodes
  this->UnitNodeComboBox->setAddEnabled(false);
  this->UnitNodeComboBox->setRemoveEnabled(false);
  this->UnitNodeComboBox->setRenameEnabled(false);
}

//-----------------------------------------------------------------------------
// qSlicerUnitsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerUnitsModuleWidget::qSlicerUnitsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerUnitsModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerUnitsModuleWidget::~qSlicerUnitsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setup()
{
  Q_D(qSlicerUnitsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::updateWidgetFromNode()
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode)
    {
    return;
    }

  d->UnitNodeComboBox->setRemoveEnabled(unitNode->GetSaveWithScene());
  d->UnitNodeComboBox->setRenameEnabled(unitNode->GetSaveWithScene());

  // Quantity
  int index = d->QuantityComboBox->findText(unitNode->GetQuantity(),
    Qt::MatchFixedString);
  if (index == -1)
    {
    // Set quantity to the current text displayd
    unitNode->SetQuantity(d->QuantityComboBox->currentText().toLatin1());
    }
  else
    {
    d->QuantityComboBox->setCurrentIndex(index);
    }

  // Suffix
  d->SuffixLineEdit->setText(unitNode->GetSuffix());

  // Prefix
  d->PrefixLineEdit->setText(QString(unitNode->GetPrefix()));

  // Precision
  d->PrecisionSpinBox->setValue(unitNode->GetPrecision());

  // Min
  d->MinimumSpinBox->setValue(unitNode->GetMinimumValue());

  // Max
  d->MaximumSpinBox->setValue(unitNode->GetMaximumValue());

  const bool builtInNode = !unitNode->GetSaveWithScene();
  d->QuantityComboBox->setEnabled(!builtInNode);
  d->PrefixLineEdit->setEnabled(!builtInNode);
  d->SuffixLineEdit->setEnabled(!builtInNode);
  d->PrecisionSpinBox->setEnabled(!builtInNode);
  d->MinimumSpinBox->setEnabled(!builtInNode);
  d->MaximumSpinBox->setEnabled(!builtInNode);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setPrefix(const QString& prefix)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene())
    {
    return;
    }

  unitNode->SetPrefix(prefix.toLatin1());

  d->MaximumSpinBox->setPrefix(prefix);
  d->MinimumSpinBox->setPrefix(prefix);
}


//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setSuffix(const QString& suffix)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene())
    {
    return;
    }

  unitNode->SetSuffix(suffix.toLatin1());

  d->MaximumSpinBox->setSuffix(suffix);
  d->MinimumSpinBox->setSuffix(suffix);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setPrecision(int newPrecision)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene())
    {
    return;
    }

  unitNode->SetPrecision(newPrecision);
  d->MaximumSpinBox->setDecimals(newPrecision);
  d->MinimumSpinBox->setDecimals(newPrecision);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::updateNodeQuantityFromWidget()
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene())
    {
    return;
    }

  unitNode->SetQuantity(d->QuantityComboBox->currentText().toLatin1());
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setMinimum(double newMin)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene())
    {
    return;
    }

  unitNode->SetMinimumValue(newMin);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setMaximum(double newMax)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene())
    {
    return;
    }

  unitNode->SetMaximumValue(newMax);
}
