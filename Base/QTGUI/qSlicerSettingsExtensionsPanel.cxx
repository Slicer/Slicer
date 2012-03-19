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

  bool RestartRequested;
};

// --------------------------------------------------------------------------
// qSlicerSettingsExtensionsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsExtensionsPanelPrivate::qSlicerSettingsExtensionsPanelPrivate(qSlicerSettingsExtensionsPanel& object)
  :q_ptr(&object)
{
  this->RestartRequested = false;
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanelPrivate::init()
{
  Q_Q(qSlicerSettingsExtensionsPanel);

  this->setupUi(q);

  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();

  // Default values
  this->ExtensionsServerUrlLineEdit->setText("http://slicer.kitware.com/midas3");
  this->ExtensionsInstallPathButton->setDirectory(coreApp->defaultExtensionsInstallPath());
#ifdef Q_OS_MAC
  this->ExtensionsInstallPathButton->setDisabled(true);
#endif

  // Register settings
  q->registerProperty("Extensions/ServerUrl", this->ExtensionsServerUrlLineEdit,
                      "text", SIGNAL(textChanged(QString)));
  q->registerProperty("Extensions/InstallPath", this->ExtensionsInstallPathButton,
                      "directory", SIGNAL(directoryChanged(QString)));

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->ExtensionsServerUrlLineEdit, SIGNAL(textChanged(QString)),
                   q, SLOT(onExensionsServerUrlChanged(QString)));
  QObject::connect(this->ExtensionsInstallPathButton, SIGNAL(directoryChanged(QString)),
                   q, SLOT(onExensionsPathChanged(QString)));

  // Hide 'Restart requested' label
  q->setRestartRequested(false);
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
qSlicerSettingsExtensionsPanel::~qSlicerSettingsExtensionsPanel()
{
}

// --------------------------------------------------------------------------
bool qSlicerSettingsExtensionsPanel::restartRequested()const
{
  Q_D(const qSlicerSettingsExtensionsPanel);
  return d->RestartRequested;
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::setRestartRequested(bool value)
{
  Q_D(qSlicerSettingsExtensionsPanel);
  d->RestartRequested = value;
  d->RestartRequestedLabel->setVisible(value);
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::resetSettings()
{
  this->Superclass::resetSettings();
  this->setRestartRequested(false);
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::onExensionsServerUrlChanged(const QString& url)
{
  Q_UNUSED(url);
}

// --------------------------------------------------------------------------
void qSlicerSettingsExtensionsPanel::onExensionsPathChanged(const QString& path)
{
  qSlicerCoreApplication::application()->setExtensionsInstallPath(path);
}
