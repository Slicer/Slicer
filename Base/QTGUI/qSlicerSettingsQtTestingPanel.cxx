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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QSettings>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsQtTestingPanel.h"
#include "ui_qSlicerSettingsQtTestingPanel.h"

// --------------------------------------------------------------------------
// qSlicerSettingsQtTestingPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsQtTestingPanelPrivate: public Ui_qSlicerSettingsQtTestingPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsQtTestingPanel);
protected:
  qSlicerSettingsQtTestingPanel* const q_ptr;

public:
  qSlicerSettingsQtTestingPanelPrivate(qSlicerSettingsQtTestingPanel& object);
  void init();

  bool RestartRequested;
};

// --------------------------------------------------------------------------
// qSlicerSettingsQtTestingPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsQtTestingPanelPrivate
::qSlicerSettingsQtTestingPanelPrivate(qSlicerSettingsQtTestingPanel& object)
  :q_ptr(&object)
{
  this->RestartRequested = false;
}

// --------------------------------------------------------------------------
void qSlicerSettingsQtTestingPanelPrivate::init()
{
  Q_Q(qSlicerSettingsQtTestingPanel);

  this->setupUi(q);

  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();

  // Default values
  this->QtTestingEnabledCheckBox->setChecked(false);

  // Register settings
  q->registerProperty("QtTesting/Enabled", this->QtTestingEnabledCheckBox,
                      "checked", SIGNAL(toggled(bool)));

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->QtTestingEnabledCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(enableQtTesting(bool)));

  // Hide 'Restart requested' label
  q->setRestartRequested(false);
}

// --------------------------------------------------------------------------
// qSlicerSettingsQtTestingPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsQtTestingPanel::qSlicerSettingsQtTestingPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsQtTestingPanelPrivate(*this))
{
  Q_D(qSlicerSettingsQtTestingPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsQtTestingPanel::~qSlicerSettingsQtTestingPanel()
{
}

// --------------------------------------------------------------------------
bool qSlicerSettingsQtTestingPanel::restartRequested()const
{
  Q_D(const qSlicerSettingsQtTestingPanel);
  return d->RestartRequested;
}

// --------------------------------------------------------------------------
void qSlicerSettingsQtTestingPanel::setRestartRequested(bool value)
{
  Q_D(qSlicerSettingsQtTestingPanel);
  d->RestartRequested = value;
  d->RestartRequestedLabel->setVisible(value);
}

// --------------------------------------------------------------------------
void qSlicerSettingsQtTestingPanel::resetSettings()
{
  this->Superclass::resetSettings();
  this->setRestartRequested(false);
}

// --------------------------------------------------------------------------
void qSlicerSettingsQtTestingPanel::enableQtTesting(bool value)
{
  bool previous = this->previousPropertyValue("QtTesting/Enabled").toBool();
  this->setRestartRequested(value != previous);
}
