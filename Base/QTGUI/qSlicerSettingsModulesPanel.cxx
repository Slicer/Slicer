/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QAction>
#include <QDebug>
#include <QMainWindow>

// CTK includes
#include <ctkLogger.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerModulesMenu.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerSettingsModulesPanel.h"
#include "ui_qSlicerSettingsModulesPanel.h"

// Default defines Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR
#include <vtkSlicerConfigure.h>

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
};

// --------------------------------------------------------------------------
// qSlicerSettingsModulesPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsModulesPanelPrivate::qSlicerSettingsModulesPanelPrivate(qSlicerSettingsModulesPanel& object)
  :q_ptr(&object)
{
  this->ModulesMenu = 0;
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanelPrivate::init()
{
  Q_Q(qSlicerSettingsModulesPanel);

  this->setupUi(q);

  this->ModulesMenu = new qSlicerModulesMenu(q);
  this->ModulesMenu->setDuplicateActions(true);
  this->HomeModuleButton->setMenu(this->ModulesMenu);
  QObject::connect(this->ModulesMenu, SIGNAL(currentModuleChanged(const QString&)),
                   q, SLOT(onHomeModuleChanged(const QString&)));
  this->ModulesMenu->setModuleManager(qSlicerCoreApplication::application()->moduleManager());

  QObject::connect(this->ShowHiddenModulesCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onShowHiddenModulesChanged(bool)));

  // default values
  this->ModulesMenu->setCurrentModule("welcome");
  this->ExtensionInstallDirectoryButton->setDirectory( qSlicerCoreApplication::application()->extensionsPath());
  this->TemporaryDirectoryButton->setDirectory( qSlicerCoreApplication::application()->temporaryPath());

  // register settings
  q->registerProperty("disable-loadable-module", this->LoadModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-cli-module", this->LoadCommandLineModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("Modules/HomeModule", this->ModulesMenu,
                      "currentModule", SIGNAL(currentModuleChanged(const QString&)));
  q->registerProperty("Modules/ExtensionsInstallDirectory", this->ExtensionInstallDirectoryButton,
                      "directory", SIGNAL(directoryChanged(const QString&)));
  q->registerProperty("Modules/TemporaryDirectory", this->TemporaryDirectoryButton,
                      "directory", SIGNAL(directoryChanged(const QString&)));
  q->registerProperty("Modules/ShowHiddenModules", this->ShowHiddenModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->ExtensionInstallDirectoryButton, SIGNAL(directoryChanged(const QString&)),
                   q, SLOT(onExensionsPathChanged(const QString&)));
  QObject::connect(this->TemporaryDirectoryButton, SIGNAL(directoryChanged(const QString&)),
                   q, SLOT(onTemporaryPathChanged(const QString&)));
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

