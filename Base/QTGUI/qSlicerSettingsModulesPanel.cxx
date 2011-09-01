/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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
#include <QMainWindow>
#include <QSettings>

// CTK includes
#include <ctkLogger.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerModulesMenu.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerSettingsModulesPanel.h"
#include "ui_qSlicerSettingsModulesPanel.h"


static ctkLogger logger("org.commontk.libs.widgets.qSlicerSettingsModulesPanel");

// --------------------------------------------------------------------------
// qSlicerSettingsModulesPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSettingsModulesPanelPrivate: public Ui_qSlicerSettingsModulesPanel
{
  Q_DECLARE_PUBLIC(qSlicerSettingsModulesPanel);
protected:
  qSlicerSettingsModulesPanel* const q_ptr;

public:
  qSlicerSettingsModulesPanelPrivate(qSlicerSettingsModulesPanel& object);
  void init();

  qSlicerModulesMenu* ModulesMenu;
  bool RestartRequested;
};

// --------------------------------------------------------------------------
// qSlicerSettingsModulesPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsModulesPanelPrivate::qSlicerSettingsModulesPanelPrivate(qSlicerSettingsModulesPanel& object)
  :q_ptr(&object)
{
  this->ModulesMenu = 0;
  this->RestartRequested = false;
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanelPrivate::init()
{
  Q_Q(qSlicerSettingsModulesPanel);

  this->setupUi(q);

  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();

  this->ModulesMenu = new qSlicerModulesMenu(q);
  this->ModulesMenu->setDuplicateActions(true);
  this->HomeModuleButton->setMenu(this->ModulesMenu);
  QObject::connect(this->ModulesMenu, SIGNAL(currentModuleChanged(QString)),
                   q, SLOT(onHomeModuleChanged(QString)));
  this->ModulesMenu->setModuleManager(coreApp->moduleManager());

  QObject::connect(this->ShowHiddenModulesCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onShowHiddenModulesChanged(bool)));

  // Default values
  this->ModulesMenu->setCurrentModule("welcome");
  this->ExtensionInstallDirectoryButton->setDirectory(coreApp->defaultExtensionsPath());
  this->TemporaryDirectoryButton->setDirectory(coreApp->defaultTemporaryPath());

  // Register settings
  q->registerProperty("disable-loadable-modules", this->LoadLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-scripted-loadable-modules", this->LoadScriptedLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-cli-modules", this->LoadCommandLineModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("Modules/HomeModule", this->ModulesMenu,
                      "currentModule", SIGNAL(currentModuleChanged(QString)));
  q->registerProperty("Modules/TemporaryDirectory", this->TemporaryDirectoryButton,
                      "directory", SIGNAL(directoryChanged(QString)));
  q->registerProperty("Modules/ShowHiddenModules", this->ShowHiddenModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("Modules/ExtensionsInstallDirectory", this->ExtensionInstallDirectoryButton,
                      "directory", SIGNAL(directoryChanged(QString)));
  q->registerProperty("Modules/AdditionalPaths", this->AdditionalModulePathsView,
                      "directoryList", SIGNAL(directoryListChanged()));

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->ExtensionInstallDirectoryButton, SIGNAL(directoryChanged(QString)),
                   q, SLOT(onExensionsPathChanged(QString)));
  QObject::connect(this->TemporaryDirectoryButton, SIGNAL(directoryChanged(QString)),
                   q, SLOT(onTemporaryPathChanged(QString)));
  QObject::connect(this->AdditionalModulePathsView, SIGNAL(directoryListChanged()),
                   q, SLOT(onAdditionalModulePathsChanged()));

  // Connect AdditionalModulePaths buttons
  QObject::connect(this->AddAdditionalModulePathButton, SIGNAL(clicked()),
                   q, SLOT(onAddModulesAdditionalPathClicked()));
  QObject::connect(this->RemoveAdditionalModulePathButton, SIGNAL(clicked()),
                   q, SLOT(onRemoveModulesAdditionalPathClicked()));

  // Hide 'Restart requested' label
  q->setRestartRequested(false);
}

// --------------------------------------------------------------------------
// qSlicerSettingsModulesPanel methods

// --------------------------------------------------------------------------
qSlicerSettingsModulesPanel::qSlicerSettingsModulesPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSettingsModulesPanelPrivate(*this))
{
  Q_D(qSlicerSettingsModulesPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSettingsModulesPanel::~qSlicerSettingsModulesPanel()
{
}

// --------------------------------------------------------------------------
bool qSlicerSettingsModulesPanel::restartRequested()const
{
  Q_D(const qSlicerSettingsModulesPanel);
  return d->RestartRequested;
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::setRestartRequested(bool value)
{
  Q_D(qSlicerSettingsModulesPanel);
  d->RestartRequested = value;
  d->RestartRequestedLabel->setVisible(value);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::resetSettings()
{
  this->Superclass::resetSettings();
  this->setRestartRequested(false);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::restoreDefaultSettings()
{
  bool shouldRestart = false;
  if (this->defaultPropertyValue("Modules/AdditionalPaths").toStringList()
      != this->previousPropertyValue("Modules/AdditionalPaths").toStringList())
    {
    shouldRestart = true;
    }
  this->Superclass::restoreDefaultSettings();
  this->setRestartRequested(shouldRestart);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onHomeModuleChanged(const QString& moduleName)
{
  Q_D(qSlicerSettingsModulesPanel);
  QAction* moduleAction = d->ModulesMenu->moduleAction(moduleName);
  Q_ASSERT(moduleAction);
  d->HomeModuleButton->setText(moduleAction->text());
  d->HomeModuleButton->setIcon(moduleAction->icon());
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onExensionsPathChanged(const QString& path)
{
  qSlicerCoreApplication::application()->setExtensionsPath(path);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onTemporaryPathChanged(const QString& path)
{
  qSlicerCoreApplication::application()->setTemporaryPath(path);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onShowHiddenModulesChanged(bool show)
{
  QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
  foreach (qSlicerModuleSelectorToolBar* toolBar,
           mainWindow->findChildren<qSlicerModuleSelectorToolBar*>())
    {
    toolBar->modulesMenu()->setShowHiddenModules(show);
    // refresh the list
    toolBar->modulesMenu()->setModuleManager(
      toolBar->modulesMenu()->moduleManager());
    }
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onAdditionalModulePathsChanged()
{
  Q_D(qSlicerSettingsModulesPanel);
  d->RemoveAdditionalModulePathButton->setEnabled(
        d->AdditionalModulePathsView->directoryList().count() > 0);
  this->setRestartRequested(true);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onAddModulesAdditionalPathClicked()
{
  Q_D(qSlicerSettingsModulesPanel);
  QString path = QFileDialog::getExistingDirectory(
        this, tr("Select folder"),
        QSettings().value("Modules/ExtensionsInstallDirectory").toString());
  // An empty directory means that the user cancelled the dialog.
  if (path.isEmpty())
    {
    return;
    }
  d->AdditionalModulePathsView->addDirectory(path);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onRemoveModulesAdditionalPathClicked()
{
  Q_D(qSlicerSettingsModulesPanel);
  // Remove all selected
  d->AdditionalModulePathsView->removeSelectedDirectories();
}

