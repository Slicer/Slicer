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
#include <QDebug>
#include <QSettings>

// CTK includes
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleManager.h"

// Slicer includes
#include "qSlicerModulesMenu.h"

class qSlicerModulesMenuPrivate
{
  Q_DECLARE_PUBLIC(qSlicerModulesMenu);
protected:
  qSlicerModulesMenu* const q_ptr;
public:
  qSlicerModulesMenuPrivate(qSlicerModulesMenu& object);
  void init();
  void addDefaultCategories();

  void addModuleAction(QMenu* menu, QAction* moduleAction, bool useIndex = true, bool builtIn = true);
  QMenu* menu(QMenu* parentMenu, QStringList subCategories, bool builtIn = true);

  bool removeTopLevelModuleAction(QAction* moduleAction);

  /// Return menus for each subCategories
  QList<QMenu*> categoryMenus(QMenu* topLevelMenu, QStringList subCategories);

  QAction* action(const QVariant& actionData, const QMenu* parentMenu)const;
  QAction* action(const QString& text, const QMenu* parentMenu)const;
  QMenu*   actionMenu(QAction* action, QMenu* parentMenu)const;

  qSlicerModuleManager* ModuleManager;
  QMenu*                AllModulesMenu;
  QAction*              NoModuleAction; //< Can be used to remove the current module
  QString               CurrentModule;
  bool                  DuplicateActions;
  bool                  ShowHiddenModules;
  bool                  AllModulesCategoryVisible;
  QStringList           TopLevelCategoryOrder;
};

//---------------------------------------------------------------------------
qSlicerModulesMenuPrivate::qSlicerModulesMenuPrivate(qSlicerModulesMenu& object)
  : q_ptr(&object)
{
  this->ModuleManager = nullptr;
  this->AllModulesMenu = nullptr;
  this->NoModuleAction = nullptr;
  this->DuplicateActions = false;
  this->ShowHiddenModules = false;
  this->AllModulesCategoryVisible = true;
  this->TopLevelCategoryOrder << "Wizards" << "Informatics" << "Registration"
    << "Segmentation" << "Quantification" << "Diffusion" << "IGT"
    << "Filtering" << "Surface Models" << "Converters" << "Endoscopy"
    << "Utilities" << "Developer Tools" << "Legacy" << "Testing";
}

//---------------------------------------------------------------------------
void qSlicerModulesMenuPrivate::init()
{
  Q_Q(qSlicerModulesMenu);
  this->AllModulesMenu = new QMenu(qSlicerModulesMenu::tr("All Modules"), q);

  this->addDefaultCategories();

  // Invisible action, don't add it anywhere.
  this->NoModuleAction = new QAction(q);
  QObject::connect(this->NoModuleAction, SIGNAL(triggered(bool)),
                   q, SLOT(onActionTriggered()));
}

//---------------------------------------------------------------------------
void qSlicerModulesMenuPrivate::addDefaultCategories()
{
  Q_Q(qSlicerModulesMenu);
  if(this->AllModulesCategoryVisible)
    {
    q->addMenu(this->AllModulesMenu);
    q->addSeparator();
    }
  // between the 2 separators are the top level modules (with no category)
  q->addSeparator();
  // after the top level modules are the non-built-in categories (from
  // extensions or any user defined external folder)
  q->addSeparator();
  // after the separator are the predefined categories
}

//---------------------------------------------------------------------------
QAction* qSlicerModulesMenuPrivate::action(const QVariant& actionData, const QMenu* parentMenu)const
{
  foreach(QAction* action, parentMenu->actions())
    {
    if (action->data() == actionData)
      {
      return action;
      }
    if (action->menu())
      {
      QAction* subAction = this->action(actionData, action->menu());
      if (subAction)
        {
        return subAction;
        }
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
QAction* qSlicerModulesMenuPrivate::action(const QString& text, const QMenu* parentMenu)const
{
  foreach(QAction* action, parentMenu->actions())
    {
    if (action->text() == text)
      {
      return action;
      }
    if (action->menu())
      {
      QAction* subAction = this->action(text, action->menu());
      if (subAction)
        {
        return subAction;
        }
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenuPrivate::addModuleAction(QMenu* menu, QAction* moduleAction, bool useIndex, bool builtIn)
{
  Q_Q(qSlicerModulesMenu);
  QList<QAction*> actions = menu->actions();
  QStringList orderedList;
  if (menu == q)
    {
    if (this->AllModulesCategoryVisible)
      {
      // special ordering at the top level, the actions need to be added
      // between the submenu AllModules and the other submenus
      actions.removeFirst(); // remove AllModules
      actions.removeFirst(); // remove first separator
      }
    if (moduleAction->menu())
      {
      orderedList = this->TopLevelCategoryOrder;
      }
    }
  // Set built-in property to action
  moduleAction->setProperty("builtIn", QVariant(builtIn));
  // The actions are before submenus and inserted based on their index or alphabetically
  bool ok = false;
  int index = moduleAction->property("index").toInt(&ok);
  if (!ok || index == -1 || !useIndex)
    {
    index = 65535; // big enough
    }
  // Search where moduleAction should be inserted. Before what action.
  foreach(QAction* action, actions)
    {
    Q_ASSERT(action);
    int actionIndex = action->property("index").toInt(&ok);
    if (!ok || actionIndex == -1 || !useIndex)
      {
      actionIndex = 65535;
      }
    bool actionBuiltIn = action->property("builtIn").toBool();
    // Sort alphabetically if the indexes are the same
    if (actionIndex == index)
      {
      if (action->text().compare(moduleAction->text(), Qt::CaseInsensitive) > 0)
        {
        actionIndex = index + 1;
        }
      }
    int order = orderedList.indexOf(moduleAction->text());
    int actionOrder = orderedList.indexOf(action->text());
    if (order != -1 || actionOrder != -1)
      {
      if (order == -1)
        {// insert (with index or alphabetically) at the end of the ordered list.
        index = actionIndex;
        }
      else if (actionOrder == -1)
        {// insert it now
        actionIndex = index + 1;
        }
      else
        {
        actionIndex = actionOrder;
        index = order;
        }
      }
    // If the action to add is NOT a menu
    if (!moduleAction->menu() && (action->menu() ||
                                  action->isSeparator() ||
                                  actionIndex > index))
      {
      menu->insertAction(action, moduleAction);
      return;
      }
    // If the action to add is a menu
    else if (moduleAction->menu() && action->menu() &&
             (actionIndex > index) && actionBuiltIn)
      {
      menu->insertAction(action, moduleAction);
      return;
      }
    }
  // if top level and not built-in, then add it to the external section
  if (menu == q && !builtIn && !orderedList.isEmpty())
    {
    // look for third separator (second, as the first one was removed above)
    int separatorCount = 0;
    foreach(QAction* action, actions)
      {
      Q_ASSERT(action);
      if (action->isSeparator())
        {
        ++separatorCount;
        if (separatorCount == 2)
          {
          menu->insertAction(action, moduleAction);
          return;
          }
        }
      }
    }
  // otherwise, simply add it to the end of the menu list
  menu->addAction(moduleAction);
}

//---------------------------------------------------------------------------
bool qSlicerModulesMenuPrivate::removeTopLevelModuleAction(QAction* moduleAction)
{
  Q_Q(qSlicerModulesMenu);
  if (!moduleAction)
    {
    return false;
    }
  QMenu* menu = this->actionMenu(moduleAction, q);
  if (!menu)
    {
    return false;
    }
  menu->removeAction(moduleAction);
  if (menu == q)
    {
    // If the action is removed from the top-level category, re-add the
    // connection.
    QObject::connect(moduleAction, SIGNAL(triggered(bool)),
                     q, SLOT(onActionTriggered()));
    }
  return true;
}

//---------------------------------------------------------------------------
QList<QMenu*> qSlicerModulesMenuPrivate::categoryMenus(QMenu* topLevelMenu, QStringList subCategories)
{
  if (subCategories.isEmpty())
    {
    return QList<QMenu*>();
    }
  QString category = subCategories.takeFirst();
  if (category.isEmpty())
    {
    return QList<QMenu*>();
    }
  foreach(QAction* action, topLevelMenu->actions())
    {
    if (action->text() == category)
      {
      QList<QMenu*> menus;
      menus.append(action->menu());
      menus.append(this->categoryMenus(action->menu(), subCategories));
      return menus;
      }
    }
  return QList<QMenu*>();
}

//---------------------------------------------------------------------------
QMenu* qSlicerModulesMenuPrivate::menu(QMenu* menu, QStringList subCategories, bool builtIn)
{
  Q_Q(qSlicerModulesMenu);
  if (subCategories.isEmpty())
    {
    return menu;
    }
  QString category = subCategories.takeFirst();
  if (category.isEmpty())
    {
    return menu;
    }
  // The actions are inserted alphabetically
  foreach(QAction* action, menu->actions())
    {
    if (action->text() == category)
      {
      return this->menu(action->menu(), subCategories);
      }
    }
  // if we are here that means the category has not been found, create it.
  QMenu* subMenu = new QMenu(category, q);
  this->addModuleAction(menu, subMenu->menuAction(), true, builtIn);
  return this->menu(subMenu, subCategories, builtIn);
}

//---------------------------------------------------------------------------
QMenu* qSlicerModulesMenuPrivate::actionMenu(QAction* action, QMenu* parentMenu)const
{
  Q_Q(const qSlicerModulesMenu);
  QList<QAction*> actions = parentMenu->actions();
  int index = actions.indexOf(action);
  if (index >= 0)
    {
    return parentMenu;
    }
  if (parentMenu == q)
    {
    actions.removeFirst();//remove All Modules menu
    }
  foreach(QAction* subAction, actions)
    {
    if (subAction->menu())
      {
      QMenu* menu = this->actionMenu(action, subAction->menu());
      if (menu)
        {
        return menu;
        }
      }
    }
  return nullptr;
}

//---------------------------------------------------------------------------
qSlicerModulesMenu::qSlicerModulesMenu(const QString& title,
                                                           QWidget* parentWidget)
  :Superclass(title, parentWidget)
  , d_ptr(new qSlicerModulesMenuPrivate(*this))
{
  Q_D(qSlicerModulesMenu);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerModulesMenu::qSlicerModulesMenu(QWidget* parentWidget)
 :Superclass(parentWidget)
  , d_ptr(new qSlicerModulesMenuPrivate(*this))
{
  Q_D(qSlicerModulesMenu);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerModulesMenu::~qSlicerModulesMenu() = default;

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setDuplicateActions(bool duplicate)
{
  Q_D(qSlicerModulesMenu);
  d->DuplicateActions = duplicate;
}

//---------------------------------------------------------------------------
bool qSlicerModulesMenu::duplicateActions()const
{
  Q_D(const qSlicerModulesMenu);
  return d->DuplicateActions;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setShowHiddenModules(bool show)
{
  Q_D(qSlicerModulesMenu);
  d->ShowHiddenModules = show;
}

//---------------------------------------------------------------------------
bool qSlicerModulesMenu::showHiddenModules()const
{
  Q_D(const qSlicerModulesMenu);
  return d->ShowHiddenModules;
}

//---------------------------------------------------------------------------
QMenu* qSlicerModulesMenu::allModulesCategory()const
{
  Q_D(const qSlicerModulesMenu);
  return d->AllModulesMenu;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setAllModulesCategoryVisible(bool visible)
{
  Q_D(qSlicerModulesMenu);
  if (d->AllModulesCategoryVisible == visible)
    {
    return;
    }
  QList<QAction*> actions = this->actions();
  if (!visible)
    {
    this->removeAction(actions.at(0)); // remove AllModules
    this->removeAction(actions.at(1)); // remove first separator
    }
  else
    {
    QAction* separator = this->insertSeparator(actions.at(0));
    this->insertMenu(separator, d->AllModulesMenu);
    }
  d->AllModulesCategoryVisible = visible;
}

//---------------------------------------------------------------------------
bool qSlicerModulesMenu::isAllModulesCategoryVisible()const
{
  Q_D(const qSlicerModulesMenu);
  return d->AllModulesCategoryVisible;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setTopLevelCategoryOrder(const QStringList& categories)
{
  Q_D(qSlicerModulesMenu);
  d->TopLevelCategoryOrder = categories;
}

//---------------------------------------------------------------------------
QStringList qSlicerModulesMenu::topLevelCategoryOrder()const
{
  Q_D(const qSlicerModulesMenu);
  return d->TopLevelCategoryOrder;
}

//---------------------------------------------------------------------------
bool qSlicerModulesMenu::removeCategory(const QString& categoryName)
{
  Q_D(qSlicerModulesMenu);
  QMenu* parentCategory = this;
  QStringList categoryNames = categoryName.split('.');
  QList<QMenu*> menus = d->categoryMenus(parentCategory, categoryNames);
  if (menus.isEmpty() || menus.count() != categoryNames.count())
    {
    return false;
    }
  QMenu* category = menus.takeLast();
  if (!menus.isEmpty())
    {
    parentCategory = menus.takeLast();
    }
  parentCategory->removeAction(category->menuAction());
  return true;
}

//---------------------------------------------------------------------------
QString qSlicerModulesMenu::currentModule()const
{
  Q_D(const qSlicerModulesMenu);
  return d->CurrentModule;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setModuleManager(qSlicerModuleManager* moduleManager)
{
  Q_D(qSlicerModulesMenu);
  if (d->ModuleManager)
    {
    QObject::disconnect(d->ModuleManager,
                        SIGNAL(moduleLoaded(QString)),
                        this, SLOT(addModule(QString)));
    QObject::disconnect(d->ModuleManager,
                        SIGNAL(moduleAboutToBeUnloaded(QString)),
                        this, SLOT(removeModule(QString)));
    }

  this->clear();
  d->addDefaultCategories();

  d->ModuleManager = moduleManager;

  if (!d->ModuleManager)
    {
    return;
    }

  QObject::connect(d->ModuleManager,
                   SIGNAL(moduleLoaded(QString)),
                   this, SLOT(addModule(QString)));
  QObject::connect(d->ModuleManager,
                   SIGNAL(moduleAboutToBeUnloaded(QString)),
                   this, SLOT(removeModule(QString)));
  this->addModules(d->ModuleManager->modulesNames());
}

//---------------------------------------------------------------------------
qSlicerModuleManager* qSlicerModulesMenu::moduleManager()const
{
  Q_D(const qSlicerModulesMenu);
  return d->ModuleManager;
}

//---------------------------------------------------------------------------
QAction* qSlicerModulesMenu::moduleAction(const QString& moduleName)const
{
  Q_D(const qSlicerModulesMenu);
  return d->action(QVariant(moduleName), d->AllModulesMenu);
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::addModule(const QString& moduleName)
{
  Q_D(qSlicerModulesMenu);
  this->addModule(d->ModuleManager ? d->ModuleManager->module(moduleName) : nullptr);
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::addModule(qSlicerAbstractCoreModule* moduleToAdd)
{
  Q_D(qSlicerModulesMenu);
  qSlicerAbstractModule* module = qobject_cast<qSlicerAbstractModule*>(moduleToAdd);
  if (!module)
    {
    qWarning() << "A module needs a QAction to be handled by qSlicerModulesMenu";
    return;
    }
  if (module->isHidden() && !d->ShowHiddenModules)
    {
    // ignore hidden modules
    return;
    }

  // Only show modules in Testing category if developer mode is enabled
  // to not clutter the module list for regular users with tests
  QSettings settings;
  bool developerModeEnabled = settings.value("Developer/DeveloperMode", false).toBool();
  if (!developerModeEnabled)
    {
    bool testOnlyModule = true;
    foreach(const QString& category, module->categories())
      {
      if (category.split('.').takeFirst()!="Testing")
        {
        testOnlyModule = false;
        }
      }
    if (testOnlyModule)
      {
      // This module only appears in the Testing category but we are not in developer mode,
      // so do not add this module to the module menu
      return;
      }
    }

  QAction* moduleAction = module->action();
  Q_ASSERT(moduleAction);
  if (d->DuplicateActions)
    {
    QAction* duplicateAction = new QAction(moduleAction->icon(), moduleAction->text(), this);
    duplicateAction->setData(moduleAction->data());
    duplicateAction->setIconVisibleInMenu(moduleAction->isIconVisibleInMenu());
    duplicateAction->setProperty("index", moduleAction->property("index"));
    moduleAction = duplicateAction;
    }
  QObject::connect(moduleAction, SIGNAL(triggered(bool)),
                   this, SLOT(onActionTriggered()));

  foreach(const QString& category, module->categories())
    {
    QMenu* menu = d->menu(this, category.split('.'), module->isBuiltIn());
    d->addModuleAction(menu, moduleAction, true, module->isBuiltIn());
    }
  // Add in "All Modules" as well
  d->addModuleAction(d->AllModulesMenu, moduleAction, false, true);

  // Maybe the module was set current before it was added into the menu
  if (d->CurrentModule == moduleAction->data().toString())
    {
    moduleAction->trigger();
    emit currentModuleChanged(d->CurrentModule);
    }
}

//---------------------------------------------------------------------------
bool qSlicerModulesMenu::removeModule(const QString& moduleName, bool removeFromAllModules)
{
   Q_D(qSlicerModulesMenu);
  return this->removeModule(d->ModuleManager ? d->ModuleManager->module(moduleName) : nullptr, removeFromAllModules);
}

//---------------------------------------------------------------------------
bool qSlicerModulesMenu::removeModule(qSlicerAbstractCoreModule* moduleToRemove, bool removeFromAllModules)
{
  Q_D(qSlicerModulesMenu);
  qSlicerAbstractModule* module = qobject_cast<qSlicerAbstractModule*>(moduleToRemove);
  if (!module)
    {
    qWarning() << "A module needs a QAction to be handled by qSlicerModulesMenu";
    return false;
    }
  QAction* moduleAction = d->action(module->action()->data(), d->AllModulesMenu);
  if (!moduleAction)
    {
    return false;
    }
  bool success = d->removeTopLevelModuleAction(moduleAction);
  if (removeFromAllModules)
    {
    d->AllModulesMenu->removeAction(moduleAction);
    success = true;
    }
  // TBD: what if the module is the current module ?
  return success;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setCurrentModule(const QString& moduleName)
{
  Q_D(qSlicerModulesMenu);
  // It's faster to look for the action in the AllModulesMenu (no need to
  // do a recursive search
  QAction* moduleAction = (!moduleName.isEmpty() ?
                           d->action(QVariant(moduleName), d->AllModulesMenu) :
                           d->NoModuleAction );
  if (!moduleAction)
    {
    // maybe the module hasn't been added yet.
    d->CurrentModule = moduleName;
    return;
    }
  // triggering the action will eventually call actionSelected();
  moduleAction->trigger();
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setCurrentModuleByTitle(const QString& title)
{
  Q_D(qSlicerModulesMenu);
  // it's faster to look for the action in the AllModulesMenu (no need to
  // do a recursive search
  QAction* moduleAction = d->action(title, d->AllModulesMenu);
  if (moduleAction)
    {
    // triggering the action will eventually call actionSelected();
    moduleAction->trigger();
    }
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::onActionTriggered()
{
  // we know for sure that the sender is the triggered QAction
  this->actionSelected(qobject_cast<QAction*>(this->sender()));
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::actionSelected(QAction* action)
{
  Q_D(qSlicerModulesMenu);
  QString newCurrentModule = action ? action->data().toString() : QString();
  if (newCurrentModule == d->CurrentModule)
    {
    return;
    }
  d->CurrentModule = newCurrentModule;
  emit currentModuleChanged(d->CurrentModule);
}
