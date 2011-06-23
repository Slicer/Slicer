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
#include <QDebug>
#include <QPixmap>
#include <QStyle>
#include <QStyleOptionButton>

// CTK includes
#include "qSlicerAbstractModule.h"
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"

// SlicerQt includes
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

  void addModuleAction(QMenu* menu, QAction* moduleAction, bool useIndex = true);
  QMenu* menu(QMenu* parentMenu, QStringList subCategories);

  QAction* action(const QVariant& actionData, const QMenu* parentMenu)const;
  QAction* action(const QString& text, const QMenu* parentMenu)const;
  QMenu*   actionMenu(QAction* action, QMenu* parentMenu)const;

  qSlicerModuleManager* ModuleManager;
  QMenu*                AllModulesMenu;
  QString               CurrentModule;
  bool                  DuplicateActions;
  bool                  ShowHiddenModules;
};

//---------------------------------------------------------------------------
qSlicerModulesMenuPrivate::qSlicerModulesMenuPrivate(qSlicerModulesMenu& object)
  : q_ptr(&object)
{
  this->ModuleManager = 0;
  this->AllModulesMenu = 0;
  this->DuplicateActions = false;
  this->ShowHiddenModules = false;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenuPrivate::init()
{
  this->addDefaultCategories();
}

//---------------------------------------------------------------------------
void qSlicerModulesMenuPrivate::addDefaultCategories()
{
  Q_Q(qSlicerModulesMenu);
  this->AllModulesMenu = q->addMenu(QObject::tr("All Modules"));
  q->addSeparator();
  // between the 2 separators goes the top level modules (with no category)
  q->addSeparator();
  q->addMenu("Wizards");
  q->addMenu("Informatics");
  q->addMenu("Registration");
  q->addMenu("Segmentation");
  q->addMenu("Quantification");
  q->addMenu("Diffusion");
  //q->addMenu("Tractography");
  q->addMenu("IGT");
  //q->addMenu("Time Series");
  q->addMenu("Filtering");
  q->addMenu("Surface Models");
  q->addMenu("Converters");
  q->addMenu("Endoscopy");
  q->addMenu("Developer Tools");
  q->addSeparator();
  // after the separator goes custom modules
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
  return 0;
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
  return 0;
}

//---------------------------------------------------------------------------
void qSlicerModulesMenuPrivate::addModuleAction(QMenu* menu, QAction* moduleAction, bool useIndex)
{
  Q_Q(qSlicerModulesMenu);
  QList<QAction*> actions = menu->actions();
  if (menu == q)
    {
    // special ordering at the top level, the actions need to be added
    // between the submenu AllModules and the other submenus
    actions.removeFirst(); // remove AllModules
    actions.removeFirst(); // remove first separator
    }
  // The actions are before submenus and inserted based on their index ot alphabetically
  bool ok = false;
  int index = moduleAction->property("index").toInt(&ok);
  if (!ok || index == -1 || !useIndex)
    {
    index = 65535; // big enough
    }
  foreach(QAction* action, actions)
    {
    Q_ASSERT(action);
    int actionIndex = action->property("index").toInt(&ok);
    if (!ok || actionIndex == -1 || !useIndex)
      {
      actionIndex = 65535;
      }
    if (!moduleAction->menu() && (action->menu() ||
                                  action->isSeparator() ||
                                  actionIndex > index ||
                                  (actionIndex == index &&
                                   action->text() > moduleAction->text())))
      {
      menu->insertAction(action, moduleAction);
      return;
      }
    else if (moduleAction->menu() && action->menu() &&
             (actionIndex > index ||
              (actionIndex == index &&
               action->text() > moduleAction->text())))
      {
      menu->insertAction(action, moduleAction);
      return;
      }
    }
  menu->addAction(moduleAction);
}

//---------------------------------------------------------------------------
QMenu* qSlicerModulesMenuPrivate::menu(QMenu* menu, QStringList subCategories)
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
  // The action are inserted alphabetically
  foreach(QAction* action, menu->actions())
    {
    if (action->text() == category)
      {
      return this->menu(action->menu(), subCategories);
      }
    }
  // if we are here that means the category has not been found, create it.
  QMenu* subMenu = new QMenu(category, q);
  this->addModuleAction(menu, subMenu->menuAction());
  return this->menu(subMenu, subCategories);
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
  return 0;
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
qSlicerModulesMenu::~qSlicerModulesMenu()
{
}

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
                        SIGNAL(moduleLoaded(qSlicerAbstractCoreModule*)),
                        this, SLOT(addModule(qSlicerAbstractCoreModule*)));
    QObject::disconnect(d->ModuleManager,
                        SIGNAL(moduleAboutToBeUnloaded(qSlicerAbstractCoreModule*)),
                        this, SLOT(removeModule(qSlicerAbstractCoreModule*)));
    }

  this->clear();
  d->addDefaultCategories();

  d->ModuleManager = moduleManager;

  if (!d->ModuleManager)
    {
    return;
    }

  QObject::connect(d->ModuleManager,
                   SIGNAL(moduleLoaded(qSlicerAbstractCoreModule*)),
                   this, SLOT(addModule(qSlicerAbstractCoreModule*)));
  QObject::connect(d->ModuleManager,
                   SIGNAL(moduleAboutToBeUnloaded(qSlicerAbstractCoreModule*)),
                   this, SLOT(removeModule(qSlicerAbstractCoreModule*)));
  this->addModules(d->ModuleManager->loadedModules());
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
  return d->action(QVariant(moduleName), this);
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::addModule(const QString& moduleName)
{
  Q_D(qSlicerModulesMenu);
  this->addModule(d->ModuleManager ? d->ModuleManager->module(moduleName) : 0);
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

  QMenu* menu = d->menu(this, module->category().split('.'));
  d->addModuleAction(menu, moduleAction);
  // Add in "All Modules" as well
  d->addModuleAction(d->AllModulesMenu, moduleAction, false);

  // Maybe the module was set current before it was added into the menu
  if (d->CurrentModule == moduleAction->data().toString())
    {
    emit currentModuleChanged(d->CurrentModule);
    }
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::removeModule(const QString& moduleName)
{
   Q_D(qSlicerModulesMenu);
  this->removeModule(d->ModuleManager ? d->ModuleManager->module(moduleName) : 0);
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::removeModule(qSlicerAbstractCoreModule* moduleToRemove)
{
  Q_D(qSlicerModulesMenu);
  qSlicerAbstractModule* module = qobject_cast<qSlicerAbstractModule*>(moduleToRemove);
  if (!module)
    {
    qWarning() << "A module needs a QAction to be handled by qSlicerModulesMenu";
    return;
    }
  QAction* moduleAction = d->action(module->action()->data(), d->AllModulesMenu);
  QMenu* menu = d->actionMenu(moduleAction, this);
  menu->removeAction(moduleAction);
  d->AllModulesMenu->removeAction(moduleAction);
  // TBD: what if the module is the current module ?
}

//---------------------------------------------------------------------------
void qSlicerModulesMenu::setCurrentModule(const QString& moduleName)
{
  Q_D(qSlicerModulesMenu);
  // It's faster to look for the action in the AllModulesMenu (no need to
  // do a recursive search
  QAction* moduleAction = d->action(QVariant(moduleName), d->AllModulesMenu);
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
