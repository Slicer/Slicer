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
#include <QMainWindow>
#include <QSettings>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerModuleFactoryFilterModel.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerModulesMenu.h"
#include "qSlicerModuleSelectorToolBar.h"
#include "qSlicerSettingsModulesPanel.h"
#include "ui_qSlicerSettingsModulesPanel.h"

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
  QStringList ModulesToAlwaysIgnore;
};

// --------------------------------------------------------------------------
// qSlicerSettingsModulesPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSettingsModulesPanelPrivate::qSlicerSettingsModulesPanelPrivate(qSlicerSettingsModulesPanel& object)
  :q_ptr(&object)
{
  this->ModulesMenu = nullptr;
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanelPrivate::init()
{
  Q_Q(qSlicerSettingsModulesPanel);

  this->setupUi(q);

  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  qSlicerAbstractModuleFactoryManager* factoryManager = coreApp->moduleManager()->factoryManager();

  // Show Hidden
  QObject::connect(this->ShowHiddenModulesCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onShowHiddenModulesChanged(bool)));

  // Additional module paths
  this->AdditionalModulePathMoreButton->setChecked(false);

  // Modules
  qSlicerModuleFactoryFilterModel* filterModel =
    this->DisableModulesListView->filterModel();
  QObject::connect(this->FilterToLoadPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowToLoad(bool)));
  QObject::connect(this->FilterToIgnorePushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowToIgnore(bool)));
  QObject::connect(this->FilterLoadedPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowLoaded(bool)));
  QObject::connect(this->FilterIgnoredPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowIgnored(bool)));
  QObject::connect(this->FilterFailedPushButton, SIGNAL(toggled(bool)),
                   filterModel, SLOT(setShowFailed(bool)));
  QObject::connect(this->FilterTitleSearchBox, SIGNAL(textChanged(QString)),
                   filterModel, SLOT(setFilterFixedString(QString)));

  this->FilterMoreButton->setChecked(false); // hide filters by default

  // Home
  this->ModulesMenu = new qSlicerModulesMenu(q);
  this->ModulesMenu->setDuplicateActions(true);
  this->HomeModuleButton->setMenu(this->ModulesMenu);
  QObject::connect(this->ModulesMenu, SIGNAL(currentModuleChanged(QString)),
                   q, SLOT(onHomeModuleChanged(QString)));
  this->ModulesMenu->setModuleManager(coreApp->moduleManager());

  // Favorites
  this->FavoritesModulesListView->filterModel()->setHideAllWhenShowModulesIsEmpty(true);
  this->FavoritesMoveLeftButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowLeft));
  this->FavoritesMoveRightButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowRight));
  QObject::connect(this->FavoritesRemoveButton, SIGNAL(clicked()),
                   this->FavoritesModulesListView, SLOT(hideSelectedModules()));
  QObject::connect(this->FavoritesMoveLeftButton, SIGNAL(clicked()),
                   this->FavoritesModulesListView, SLOT(moveLeftSelectedModules()));
  QObject::connect(this->FavoritesMoveRightButton, SIGNAL(clicked()),
                   this->FavoritesModulesListView, SLOT(moveRightSelectedModules()));
  QObject::connect(this->FavoritesMoreButton, SIGNAL(toggled(bool)),
                   this->FavoritesModulesListView, SLOT(scrollToSelectedModules()));
  this->FavoritesMoreButton->setChecked(false);

  // Default values
  this->PreferExecutableCLICheckBox->setChecked(Slicer_CLI_PREFER_EXECUTABLE_DEFAULT);
  this->TemporaryDirectoryButton->setDirectory(coreApp->defaultTemporaryPath());
  this->DisableModulesListView->setFactoryManager( factoryManager );
  this->FavoritesModulesListView->setFactoryManager( factoryManager );
  
  this->ModulesMenu->setCurrentModule(Slicer_DEFAULT_HOME_MODULE);
  
  // Slicer_DEFAULT_FAVORITE_MODULES contains module names in a comma-separated list 
  // (chosen this format because the same format is used for storing the favorites list in the .ini file).
  QStringList favoritesRaw = QString(Slicer_DEFAULT_FAVORITE_MODULES).split(",", QString::SkipEmptyParts);
  // The separator commas have been removed, but we also need need to remove leading and trailing spaces from the retrieved names.
  QStringList favorites;
  foreach(QString s, favoritesRaw)
    {
    favorites << s.trimmed();
    }
  this->FavoritesModulesListView->filterModel()->setShowModules(favorites);

  // Register settings
  q->registerProperty("disable-loadable-modules", this->LoadLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-scripted-loadable-modules", this->LoadScriptedLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-cli-modules", this->LoadCommandLineModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));

  q->registerProperty("disable-builtin-loadable-modules", this->LoadBuiltInLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-builtin-scripted-loadable-modules", this->LoadBuiltInScriptedLoadableModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("disable-builtin-cli-modules", this->LoadBuiltInCommandLineModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));

  q->registerProperty("Modules/PreferExecutableCLI", this->PreferExecutableCLICheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("Modules/HomeModule", this->ModulesMenu,
                      "currentModule", SIGNAL(currentModuleChanged(QString)));
  q->registerProperty("Modules/FavoriteModules", this->FavoritesModulesListView->filterModel(),
                      "showModules", SIGNAL(showModulesChanged(QStringList)));
  q->registerProperty("Modules/TemporaryDirectory", this->TemporaryDirectoryButton,
                      "directory", SIGNAL(directoryChanged(QString)));
  q->registerProperty("Modules/ShowHiddenModules", this->ShowHiddenModulesCheckBox,
                      "checked", SIGNAL(toggled(bool)));
  q->registerProperty("Modules/AdditionalPaths", this->AdditionalModulePathsView,
                      "directoryList", SIGNAL(directoryListChanged()),
                      "Additional module paths", ctkSettingsPanel::OptionRequireRestart,
                      coreApp->revisionUserSettings());

  this->ModulesToAlwaysIgnore = coreApp->revisionUserSettings()->value("Modules/IgnoreModules").toStringList();
  emit q->modulesToAlwaysIgnoreChanged(this->ModulesToAlwaysIgnore);

  q->registerProperty("Modules/IgnoreModules", q,
                      "modulesToAlwaysIgnore", SIGNAL(modulesToAlwaysIgnoreChanged(QStringList)),
                      "Modules to ignore", ctkSettingsPanel::OptionRequireRestart,
                      coreApp->revisionUserSettings());
  QObject::connect(factoryManager, SIGNAL(modulesToIgnoreChanged(QStringList)),
                   q, SLOT(setModulesToAlwaysIgnore(QStringList)));

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->TemporaryDirectoryButton, SIGNAL(directoryChanged(QString)),
                   q, SLOT(onTemporaryPathChanged(QString)));
  QObject::connect(this->AdditionalModulePathsView, SIGNAL(directoryListChanged()),
                   q, SLOT(onAdditionalModulePathsChanged()));

  // Connect AdditionalModulePaths buttons
  QObject::connect(this->AddAdditionalModulePathButton, SIGNAL(clicked()),
                   q, SLOT(onAddModulesAdditionalPathClicked()));
  QObject::connect(this->RemoveAdditionalModulePathButton, SIGNAL(clicked()),
                   q, SLOT(onRemoveModulesAdditionalPathClicked()));
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
qSlicerSettingsModulesPanel::~qSlicerSettingsModulesPanel() = default;

//-----------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::setModulesToAlwaysIgnore(const QStringList& moduleNames)
{
  Q_D(qSlicerSettingsModulesPanel);

  // This slot is called in two cases:
  //
  // (1) each time the signal qSlicerAbstractModuleFactoryManager::modulesToIgnore
  // is invoked.
  //
  // (2) each time the default settings are restored.
  //
  // To ensure the module names specified using the "--modules-to-ignore"
  // command line arguments are not saved in the settings, this  slot
  // will emit the "onModulesToAlwaysIgnoreChanged()" with an updated
  // list.

  if (d->ModulesToAlwaysIgnore == moduleNames)
    {
    return;
    }

  // Ensure the ModulesListView observing the factoryManager is updated
  // when settings are restored.
  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  coreApp->moduleManager()->factoryManager()->setModulesToIgnore(moduleNames);

  // Update the list of modules to ignore removing the one
  // specified from the command line.
  QStringList updatedModulesToAlwaysIgnore;
  foreach(const QString& moduleName, moduleNames)
    {
    if (!coreApp->coreCommandOptions()->modulesToIgnore().contains(moduleName))
      {
      updatedModulesToAlwaysIgnore.append(moduleName);
      }
    }

  if (d->ModulesToAlwaysIgnore == updatedModulesToAlwaysIgnore)
    {
    return;
    }

  d->ModulesToAlwaysIgnore = updatedModulesToAlwaysIgnore;

  emit modulesToAlwaysIgnoreChanged(updatedModulesToAlwaysIgnore);
}

//-----------------------------------------------------------------------------
QStringList qSlicerSettingsModulesPanel::modulesToAlwaysIgnore()const
{
  Q_D(const qSlicerSettingsModulesPanel);
  return d->ModulesToAlwaysIgnore;
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
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onAddModulesAdditionalPathClicked()
{
  Q_D(qSlicerSettingsModulesPanel);
  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  QString path = QFileDialog::getExistingDirectory(
        this, tr("Select folder"),
        coreApp->revisionUserSettings()->value("Modules/MostRecentlySelectedPath").toString());
  // An empty directory means that the user cancelled the dialog.
  if (path.isEmpty())
    {
    return;
    }
  d->AdditionalModulePathsView->addDirectory(path);
  coreApp->revisionUserSettings()->setValue("Modules/MostRecentlySelectedPath", path);
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onRemoveModulesAdditionalPathClicked()
{
  Q_D(qSlicerSettingsModulesPanel);
  // Remove all selected
  d->AdditionalModulePathsView->removeSelectedDirectories();
}

