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

  This file was originally developed by Benjamin Long, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QSettings>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSettingsInternationalizationPanel.h"
#include "ui_qSlicerSettingsInternationalizationPanel.h"

// --------------------------------------------------------------------------
// qSlicerSettingsInternationalizationPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsInternationalizationPanelPrivate: public Ui_qSlicerSettingsInternationalizationPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsInternationalizationPanel);
protected:
  qSlicerSettingsInternationalizationPanel* const q_ptr;

public:
  qSlicerSettingsInternationalizationPanelPrivate(qSlicerSettingsInternationalizationPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qSlicerSettingsInternationalizationPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsInternationalizationPanelPrivate
::qSlicerSettingsInternationalizationPanelPrivate(qSlicerSettingsInternationalizationPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsInternationalizationPanelPrivate::init()
{
  Q_Q(qSlicerSettingsInternationalizationPanel);

  this->setupUi(q);

  // Default values
  this->InternationalizationEnabledCheckBox->setChecked(false);

  // Register settings
  q->registerProperty("Internationalization/Enabled",
                      this->InternationalizationEnabledCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Enable/Disable Internationalization",
                      ctkSettingsPanel::OptionRequireRestart);

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->InternationalizationEnabledCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(enableInternationalization(bool)));
}

// --------------------------------------------------------------------------
// qSlicerSettingsInternationalizationPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsInternationalizationPanel::qSlicerSettingsInternationalizationPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsInternationalizationPanelPrivate(*this))
{
  Q_D(qSlicerSettingsInternationalizationPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsInternationalizationPanel::~qSlicerSettingsInternationalizationPanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsInternationalizationPanel::enableInternationalization(bool value)
{
  Q_UNUSED(value);
}
