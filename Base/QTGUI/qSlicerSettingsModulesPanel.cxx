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
  this->PreferExecutableCLICheckBox->setChecked(false);
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
  q->registerProperty("Modules/IgnoreModules", factoryManager,
                      "modulesToIgnore", SIGNAL(modulesToIgnoreChanged(QStringList)),
                      "Modules to ignore", ctkSettingsPanel::OptionRequireRestart,
                      coreApp->revisionUserSettings());

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

  // Connect Modules to ignore
  QObject::connect(factoryManager, SIGNAL(modulesToIgnoreChanged(QStringList)),
                   q, SLOT(onModulesToIgnoreChanged()));
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
void qSlicerSettingsModulesPanel::onModulesToIgnoreChanged()
{
}

// --------------------------------------------------------------------------
void qSlicerSettingsModulesPanel::onAddModulesAdditionalPathClicked()
{
  Q_D(qSlicerSettingsModulesPanel);
  qSlicerCoreApplication * coreApp = qSlicerCoreApplication::application();
  QString path = QFileDialog::getExistingDirectory(
        this, tr("Select folder"),
        coreApp->revisionUserSettings()->value("Extensions/InstallPath").toString());
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

