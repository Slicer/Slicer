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
#include <QWidget>

// Slicer includes
#include "qMRMLSettingsUnitWidget.h"
#include "qMRMLUnitWidget.h"
#include "ui_qMRMLSettingsUnitWidget.h"

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLUnitNode.h"

// STD
#include <vector>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qMRMLSettingsUnitWidgetPrivate: public Ui_qMRMLSettingsUnitWidget
{
  Q_DECLARE_PUBLIC(qMRMLSettingsUnitWidget);
protected:
  qMRMLSettingsUnitWidget* const q_ptr;

public:
  qMRMLSettingsUnitWidgetPrivate(qMRMLSettingsUnitWidget& obj);
  void setupUi(qMRMLSettingsUnitWidget*);

  vtkSlicerUnitsLogic* Logic;
};

//-----------------------------------------------------------------------------
// qMRMLSettingsUnitWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLSettingsUnitWidgetPrivate::qMRMLSettingsUnitWidgetPrivate(
  qMRMLSettingsUnitWidget& object)
  : q_ptr(&object)
{
  this->Logic = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLSettingsUnitWidgetPrivate::setupUi(qMRMLSettingsUnitWidget* q)
{
  this->Ui_qMRMLSettingsUnitWidget::setupUi(q);

  QObject::connect(this->UnitNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this->UnitInfoWidget, SLOT(setCurrentNode(vtkMRMLNode*)));

  // Hide unit label and combobox for now
  this->UnitLabel->setVisible(false);
  this->UnitNodeComboBox->setVisible(false);
}

//-----------------------------------------------------------------------------
// qMRMLSettingsUnitWidget methods

//-----------------------------------------------------------------------------
qMRMLSettingsUnitWidget::qMRMLSettingsUnitWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qMRMLSettingsUnitWidgetPrivate(*this) )
{
  Q_D(qMRMLSettingsUnitWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qMRMLSettingsUnitWidget::~qMRMLSettingsUnitWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLSettingsUnitWidget::setUnitsLogic(vtkSlicerUnitsLogic* logic)
{
  Q_D(qMRMLSettingsUnitWidget);
  if (logic == d->Logic)
    {
    return;
    }

  d->Logic = logic;
  d->UnitInfoWidget->setMRMLScene(d->Logic ? d->Logic->GetUnitsScene() : nullptr);
}

//-----------------------------------------------------------------------------
qMRMLNodeComboBox* qMRMLSettingsUnitWidget::unitComboBox()
{
  Q_D(qMRMLSettingsUnitWidget);
  return d->UnitNodeComboBox;
}

//-----------------------------------------------------------------------------
qMRMLUnitWidget* qMRMLSettingsUnitWidget::unitWidget()
{
  Q_D(qMRMLSettingsUnitWidget);
  return d->UnitInfoWidget;
}
