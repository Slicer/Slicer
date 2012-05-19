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
};

// --------------------------------------------------------------------------
// qSlicerSettingsQtTestingPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsQtTestingPanelPrivate
::qSlicerSettingsQtTestingPanelPrivate(qSlicerSettingsQtTestingPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsQtTestingPanelPrivate::init()
{
  Q_Q(qSlicerSettingsQtTestingPanel);

  this->setupUi(q);

  // Default values
  this->QtTestingEnabledCheckBox->setChecked(false);

  // Register settings
  q->registerProperty("QtTesting/Enabled", this->QtTestingEnabledCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Enable/Disable QtTesting", ctkSettingsPanel::OptionRequireRestart);

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->QtTestingEnabledCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(enableQtTesting(bool)));
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
void qSlicerSettingsQtTestingPanel::enableQtTesting(bool value)
{
  Q_UNUSED(value);
}
