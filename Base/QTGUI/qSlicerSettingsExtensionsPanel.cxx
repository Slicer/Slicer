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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QMainWindow>
#include <QSettings>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerSettingsExtensionsPanel.h"
#include "ui_qSlicerSettingsExtensionsPanel.h"

// --------------------------------------------------------------------------
// qSlicerSettingsExtensionsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsExtensionsPanelPrivate: public Ui_qSlicerSettingsExtensionsPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsExtensionsPanel);
protected:
  qSlicerSettingsExtensionsPanel* const q_ptr;

public:
  qSlicerSettingsExtensionsPanelPrivate(qSlicerSettingsExtensionsPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qSlicerSettingsExtensionsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsExtensionsPanelPrivate::qSlicerSettingsExtensionsPanelPrivate(qSlicerSettingsExtensionsPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanelPrivate::init()
{
  Q_Q(qSlicerSettingsExtensionsPanel);

  this->setupUi(q);

  qSlicerApplication * app = qSlicerApplication::application();

  // Default values
  this->ExtensionsManagerEnabledCheckBox->setChecked(true);
  this->ExtensionsServerUrlLineEdit->setText("http://slicer.kitware.com/midas3");
  this->ExtensionsInstallPathButton->setDirectory(app->defaultExtensionsInstallPath());
#ifdef Q_OS_MAC
  this->ExtensionsInstallPathButton->setDisabled(true);
#endif

  // Register settings
  q->registerProperty("Extensions/ManagerEnabled", this->ExtensionsManagerEnabledCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Enable/Disable extension manager", ctkSettingsPanel::OptionRequireRestart,
                      app->revisionUserSettings());
  q->registerProperty("Extensions/ServerUrl", this->ExtensionsServerUrlLineEdit,
                      "text", SIGNAL(textChanged(QString)),
                      QString(), ctkSettingsPanel::OptionNone,
                      app->revisionUserSettings());
  q->registerProperty("Extensions/InstallPath", this->ExtensionsInstallPathButton,
                      "directory", SIGNAL(directoryChanged(QString)),
                      QString(), ctkSettingsPanel::OptionNone,
                      app->revisionUserSettings());

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->ExtensionsManagerEnabledCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onExtensionsManagerEnabled(bool)));
  QObject::connect(this->ExtensionsServerUrlLineEdit, SIGNAL(textChanged(QString)),
                   q, SIGNAL(extensionsServerUrlChanged(QString)));
  QObject::connect(this->ExtensionsInstallPathButton, SIGNAL(directoryChanged(QString)),
                   q, SLOT(onExensionsPathChanged(QString)));
  QObject::connect(this->OpenExtensionsManagerPushButton, SIGNAL(clicked()),
                   app, SLOT(openExtensionsManagerDialog()));
}

// --------------------------------------------------------------------------
// qSlicerSettingsExtensionsPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsExtensionsPanel::qSlicerSettingsExtensionsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsExtensionsPanelPrivate(*this))
{
  Q_D(qSlicerSettingsExtensionsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsExtensionsPanel::~qSlicerSettingsExtensionsPanel() = default;

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::onExtensionsManagerEnabled(bool value)
{
  Q_UNUSED(value);
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::onExensionsPathChanged(const QString& path)
{
  qSlicerCoreApplication::application()->setExtensionsInstallPath(path);
}
