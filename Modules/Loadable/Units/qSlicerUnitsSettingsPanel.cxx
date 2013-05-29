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
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QHash>
#include <QSettings>

// QtGUI includes
#include "qMRMLNodeComboBox.h"
#include "qSlicerApplication.h"
#include "qSlicerUnitsSettingsPanel.h"
#include "ui_qSlicerUnitsSettingsPanel.h"

// Units includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLUnitNode.h"
#include "vtkSlicerUnitsLogic.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// --------------------------------------------------------------------------
// qSlicerUnitsSettingsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerUnitsSettingsPanelPrivate: public Ui_qSlicerUnitsSettingsPanel
{
  Q_DECLARE_PUBLIC(qSlicerUnitsSettingsPanel);
protected:
  qSlicerUnitsSettingsPanel* const q_ptr;

public:
  qSlicerUnitsSettingsPanelPrivate(qSlicerUnitsSettingsPanel& object);
  void init();

  vtkSmartPointer<vtkSlicerUnitsLogic> Logic;

  void udpatePanel();
  void registerProperties();
};

// --------------------------------------------------------------------------
// qSlicerUnitsSettingsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerUnitsSettingsPanelPrivate
::qSlicerUnitsSettingsPanelPrivate(qSlicerUnitsSettingsPanel& object)
  :q_ptr(&object)
{
  this->Logic = 0;
}

// --------------------------------------------------------------------------
void qSlicerUnitsSettingsPanelPrivate::init()
{
  Q_Q(qSlicerUnitsSettingsPanel);

  this->setupUi(q);

  this->LengthComboBox->addAttribute("vtkMRMLUnitNode", "Quantity", "length");
  // \todo get quantiy from proxy instead if possible
  this->LengthComboBox->setProperty("Quantity", "length");
  this->TimeComboBox->addAttribute("vtkMRMLUnitNode", "Quantity", "time");
  this->TimeComboBox->setProperty("Quantity", "time");

  q->connect(this->LengthComboBox,
    SIGNAL(currentNodeIDChanged(QString)),
    q, SLOT(onNodeIDChanged(QString)));
  q->connect(this->TimeComboBox,
    SIGNAL(currentNodeIDChanged(QString)),
    q, SLOT(onNodeIDChanged(QString)));
}

// --------------------------------------------------------------------------
void qSlicerUnitsSettingsPanelPrivate::udpatePanel()
{
  Q_Q(qSlicerUnitsSettingsPanel);
  vtkMRMLScene* scene = 0;
  if (this->Logic)
    {
    scene = this->Logic->GetMRMLScene();
    }

  this->LengthComboBox->setMRMLScene(scene);
  this->TimeComboBox->setMRMLScene(scene);
  if (scene)
    {
    this->registerProperties();
    }
}

// ---------------------------------------------------------------------------
void qSlicerUnitsSettingsPanelPrivate::registerProperties()
{
  Q_Q(qSlicerUnitsSettingsPanel);
  q->registerProperty("Units/length", this->LengthComboBox, "currentNodeID",
    SIGNAL(currentNodeIDChanged(QString)));
  q->registerProperty("Units/time", this->TimeComboBox, "currentNodeID",
    SIGNAL(currentNodeIDChanged(QString)));
}

// --------------------------------------------------------------------------
// qSlicerUnitsSettingsPanel methods

// --------------------------------------------------------------------------
qSlicerUnitsSettingsPanel::qSlicerUnitsSettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerUnitsSettingsPanelPrivate(*this))
{
  Q_D(qSlicerUnitsSettingsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerUnitsSettingsPanel::~qSlicerUnitsSettingsPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerUnitsSettingsPanel::setUnitsLogic(vtkSlicerUnitsLogic* logic)
{
  Q_D(qSlicerUnitsSettingsPanel);

  qvtkReconnect(d->Logic, logic, vtkCommand::ModifiedEvent,
                this, SLOT(onUnitsLogicModified()));
  d->Logic = logic;

  this->onUnitsLogicModified();
}

// --------------------------------------------------------------------------
void qSlicerUnitsSettingsPanel::onNodeIDChanged(const QString& id)
{
  Q_D(qSlicerUnitsSettingsPanel);
  if (d->Logic)
    {
    qMRMLNodeComboBox* sender =
      qobject_cast<qMRMLNodeComboBox*>(QObject::sender());
    Q_ASSERT(sender);

    QString quantity = sender->property("Quantity").toString();
    d->Logic->SetDefaultUnit(quantity.toLatin1(), id.toLatin1());
    }
}

// --------------------------------------------------------------------------
void qSlicerUnitsSettingsPanel::onUnitsLogicModified()
{
  Q_D(qSlicerUnitsSettingsPanel);
  d->udpatePanel();
}
