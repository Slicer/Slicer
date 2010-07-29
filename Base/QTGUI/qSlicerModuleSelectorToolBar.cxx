
// Qt includes
#include <QCompleter>
#include <QDebug>
#include <QFontMetrics>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
#include <QStyleOptionButton>
#include <QToolButton>

// CTK includes
#include "qSlicerAbstractModule.h"
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"

// SlicerQt includes
#include "qSlicerModuleSelectorToolBar.h"

class qSlicerModuleSelectorToolBarPrivate:public ctkPrivate<qSlicerModuleSelectorToolBar>
{
public:
  qSlicerModuleSelectorToolBarPrivate();
  void init();
  void addDefaultCategories();

  void addModuleAction(QMenu* menu, QAction* moduleAction);
  QMenu* menu(QMenu* parentMenu, QStringList subCategories);
  /// Return the last action in the menu that is not a menu (recursive)
  QAction* lastMenuAction(QMenu* menu)const;
  QAction* firstMenuAction(QMenu* menu)const;

  QAction* action(const QVariant& actionData, QMenu* parentMenu)const;
  QAction* action(const QString& text, QMenu* parentMenu)const;
  QAction* lastSelectedAction()const;
  QAction* previousAction(QAction* action)const;
  QAction* nextAction(QAction* action)const;
  QMenu*   actionMenu(QAction* action, QMenu* parentMenu)const;

  void insertActionInHistory(QAction* action);

  QMenu*       AllModulesMenu;
  QMenu*       ModulesMenu;
  QPushButton* ModulesButton;
  QMenu*       HistoryMenu;
  QToolButton* HistoryButton;
  QLineEdit*   ModuleSearchLineEdit;
  QCompleter*  ModuleSearchCompleter;
};


//---------------------------------------------------------------------------
qSlicerModuleSelectorToolBarPrivate::qSlicerModuleSelectorToolBarPrivate()
{
  this->AllModulesMenu = 0;
  this->ModulesMenu = 0;
  this->ModulesButton = 0;
  this->HistoryMenu = 0;
  this->HistoryButton = 0;
  this->ModuleSearchLineEdit = 0;
  this->ModuleSearchCompleter = 0;
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBarPrivate::init()
{
  CTK_P(qSlicerModuleSelectorToolBar);
  QIcon previousIcon = p->style()->standardIcon(QStyle::SP_ArrowLeft);
  QIcon nextIcon = p->style()->standardIcon(QStyle::SP_ArrowRight);
  QIcon historyIcon(":Icons/ModuleHistory.png");

  // Modules Label
  p->addWidget(new QLabel("Modules:", p));

  // Modules pushbutton
  this->ModulesMenu = new QMenu("Modules",p);
  this->ModulesButton = new QPushButton(p);
  this->ModulesButton->setMenu(this->ModulesMenu);
  QStyleOptionButton opt;
  this->ModulesButton->setFixedWidth(
    this->ModulesButton->style()->sizeFromContents(
      QStyle::CT_PushButton, &opt,
      this->ModulesButton->fontMetrics().size(
        Qt::TextShowMnemonic, "XXXXXXXXXXXXXXXXXXXXXXXXXX"),this->ModulesButton).width());
  p->addWidget(this->ModulesButton);
  QObject::connect(this->ModulesMenu, SIGNAL(triggered(QAction*)),
                   p, SLOT(actionSelected(QAction*)));

  // Previous / Next
  p->addAction(previousIcon, "Previous", p, SLOT(selectPreviousModule()));
  p->addAction(nextIcon, "Next", p, SLOT(selectNextModule()));

  // History
  this->HistoryMenu = new QMenu("Modules history", p);
  this->HistoryButton = new QToolButton;
  this->HistoryButton->setIcon(historyIcon);
  this->HistoryButton->setMenu(this->HistoryMenu);
  this->HistoryButton->setPopupMode(QToolButton::InstantPopup);
  p->addWidget(this->HistoryButton);

  // Search
  this->ModuleSearchLineEdit = new QLineEdit(p);
  // TODO clear the search text when the line edit gets the focus
  this->ModuleSearchLineEdit->setText("Search a module");
  this->ModuleSearchCompleter = new QCompleter(QStringList());
  this->ModuleSearchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  this->ModuleSearchLineEdit->setCompleter(this->ModuleSearchCompleter);
  QObject::connect(this->ModuleSearchLineEdit, SIGNAL(returnPressed()),
                   p, SLOT(searchModule()));
  p->addWidget(this->ModuleSearchLineEdit);

  this->addDefaultCategories();
}

void qSlicerModuleSelectorToolBarPrivate::addDefaultCategories()
{
  this->AllModulesMenu = this->ModulesMenu->addMenu("All Modules");
  this->ModulesMenu->addSeparator();
  // between the 2 separators goes the top level modules (with no category)
  this->ModulesMenu->addSeparator();
  this->ModulesMenu->addMenu("Wizards");
  this->ModulesMenu->addMenu("Informatics");
  this->ModulesMenu->addMenu("Registration");
  this->ModulesMenu->addMenu("Segmentation");
  this->ModulesMenu->addMenu("Quantification");
  this->ModulesMenu->addMenu("Diffusion");
  this->ModulesMenu->addMenu("Tractography");
  this->ModulesMenu->addMenu("IGT");
  this->ModulesMenu->addMenu("Time Series");
  this->ModulesMenu->addMenu("Filtering");
  this->ModulesMenu->addMenu("Surface Models");
  this->ModulesMenu->addMenu("Converters");
  this->ModulesMenu->addMenu("Endoscopy");
  this->ModulesMenu->addMenu("Developer Tools");
  this->ModulesMenu->addSeparator();
  // after the separator goes custom modules
}

//---------------------------------------------------------------------------
QAction* qSlicerModuleSelectorToolBarPrivate::action(const QVariant& actionData, QMenu* parentMenu)const
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
QAction* qSlicerModuleSelectorToolBarPrivate::action(const QString& text, QMenu* parentMenu)const
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
void qSlicerModuleSelectorToolBarPrivate::addModuleAction(QMenu* menu, QAction* moduleAction)
{
  QList<QAction*> actions = menu->actions();
  if (menu == this->ModulesMenu)
    {
    // special ordering at the top level, the actions need to be added
    // between the submenu AllModules and the other submenus
    actions.removeFirst(); // remove AllModules
    actions.removeFirst(); // remove first separator
    }
  // The actions are before submenus and inserted alphabetically
  foreach(QAction* action, actions)
    {
    if (!moduleAction->menu() && (action->menu() ||
                                  action->isSeparator() ||
                                  action->text() > moduleAction->text()))
      {
      menu->insertAction(action, moduleAction);
      return;
      }
    else if (moduleAction->menu() && action->menu() &&
             action->text() > moduleAction->text())
      {
      menu->insertAction(action, moduleAction);
      return;
      }
    }
  menu->addAction(moduleAction);
}

//---------------------------------------------------------------------------
QMenu* qSlicerModuleSelectorToolBarPrivate::menu(QMenu* menu, QStringList subCategories)
{
  CTK_P(qSlicerModuleSelectorToolBar);
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
  QMenu* subMenu = new QMenu(category, p);
  this->addModuleAction(menu, subMenu->menuAction());
  return this->menu(subMenu, subCategories);
}

//---------------------------------------------------------------------------
QAction* qSlicerModuleSelectorToolBarPrivate::firstMenuAction(QMenu* menu)const
{
  QList<QAction*> actions = menu->actions();
  if (actions.isEmpty())
    {
    return 0;
    }
  QAction* firstAction = actions[0];
  if (firstAction->menu())
    {
    return this->firstMenuAction(firstAction->menu());
    }
  return firstAction;
}

//---------------------------------------------------------------------------
QAction* qSlicerModuleSelectorToolBarPrivate::lastMenuAction(QMenu* menu)const
{
  QList<QAction*> actions = menu->actions();
  if (actions.isEmpty())
    {
    return 0;
    }
  QAction* lastAction = actions[actions.size()-1];
  if (lastAction->menu())
    {
    return this->lastMenuAction(lastAction->menu());
    }
  return lastAction;
}

//---------------------------------------------------------------------------
QAction* qSlicerModuleSelectorToolBarPrivate::lastSelectedAction()const
{
  QList<QAction*> actions = this->HistoryMenu->actions();
  return actions.size() ? actions[0] : 0;
}

//---------------------------------------------------------------------------
QAction* qSlicerModuleSelectorToolBarPrivate::previousAction(QAction* action)const
{
  QMenu* menu = this->AllModulesMenu;
  QList<QAction*> actions = menu->actions();
  int index = actions.indexOf(action);
  Q_ASSERT(actions.size());
  if (index < 0)
    {
    // error
    return actions[0];
    }
  QAction* previousAction = 0;
  if (index == 0)
    {
    previousAction = this->AllModulesMenu->menuAction();
    }
  else
    {
    previousAction = actions[index - 1];
    }
  if (previousAction->menu())
    {
    previousAction = this->lastMenuAction(previousAction->menu());
    }
  return previousAction;
}

//---------------------------------------------------------------------------
QAction* qSlicerModuleSelectorToolBarPrivate::nextAction(QAction* action)const
{
  QMenu* menu = this->AllModulesMenu;
  QList<QAction*> actions = menu->actions();
  int index = actions.indexOf(action);
  Q_ASSERT(actions.size());
  if (index < 0)
    {
    // error
    return actions[0];
    }
  QAction* nextAction = 0;
  if (index == actions.size()-1)
    {
    nextAction = this->AllModulesMenu->menuAction();
    }
  else
    {
    nextAction = actions[index + 1];
    }
  if (nextAction->menu())
    {
    nextAction = this->firstMenuAction(nextAction->menu());
    }
  return nextAction;
}

//---------------------------------------------------------------------------
QMenu* qSlicerModuleSelectorToolBarPrivate::actionMenu(QAction* action, QMenu* parentMenu)const
{
  QList<QAction*> actions = parentMenu->actions();
  int index = actions.indexOf(action);
  if (index >= 0)
    {
    return parentMenu;
    }
  if (parentMenu == this->ModulesMenu)
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
void qSlicerModuleSelectorToolBarPrivate::insertActionInHistory(QAction* action)
{
  this->HistoryMenu->removeAction(action);
  QAction* before = this->HistoryMenu->actions().isEmpty() ? 0 : this->HistoryMenu->actions().first();
  this->HistoryMenu->insertAction(before, action);
  QList<QAction*> actions = this->HistoryMenu->actions();
  for (int i = 8; i < actions.size(); ++i)
    {
    this->HistoryMenu->removeAction(actions[i]);
    }
}

//---------------------------------------------------------------------------
qSlicerModuleSelectorToolBar::qSlicerModuleSelectorToolBar(const QString& title,
                                                           QWidget* parentWidget)
  :Superclass(title, parentWidget)
{
  CTK_INIT_PRIVATE(qSlicerModuleSelectorToolBar);
  CTK_D(qSlicerModuleSelectorToolBar);
  d->init();
}

//---------------------------------------------------------------------------
qSlicerModuleSelectorToolBar::qSlicerModuleSelectorToolBar(QWidget* parentWidget)
 :Superclass(parentWidget)
{
  CTK_INIT_PRIVATE(qSlicerModuleSelectorToolBar);
  CTK_D(qSlicerModuleSelectorToolBar);
  d->init();
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::addModule(const QString& moduleName)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  qSlicerAbstractModule* module = qobject_cast<qSlicerAbstractModule*>(
    qSlicerApplication::application()->moduleManager()->module(moduleName));
  if (!module)
    {
    return;
    }
  QAction* moduleAction = module->action();

  QMenu* menu = d->menu(d->ModulesMenu, module->category().split('.'));
  d->addModuleAction(menu, moduleAction);
  // Add in "All Modules
  d->addModuleAction(d->AllModulesMenu, moduleAction);

  // here we assume the model is not automatically sorted
  int actionCount = d->ModuleSearchCompleter->model()->rowCount();
  d->ModuleSearchCompleter->model()->insertRow(actionCount);
  QModelIndex index = d->ModuleSearchCompleter->model()->index(actionCount -1, 0);
  d->ModuleSearchCompleter->model()->setData(index, module->title());
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::removeModule(const QString& moduleName)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  QAction* moduleAction = d->action(QVariant(moduleName), d->ModulesMenu);
  QMenu* menu = d->actionMenu(moduleAction, d->ModulesMenu);
  menu->removeAction(moduleAction);
  d->AllModulesMenu->removeAction(moduleAction);
  d->HistoryMenu->removeAction(moduleAction);
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectModule(const QString& moduleName)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  QAction* moduleAction = d->action(QVariant(moduleName), d->ModulesMenu);
  if (moduleAction)
    {
    this->actionSelected(moduleAction);
    }
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectModuleByTitle(const QString& title)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  QAction* moduleAction = d->action(title, d->ModulesMenu);
  if (moduleAction)
    {
    this->actionSelected(moduleAction);
    }
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::actionSelected(QAction* action)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  d->ModulesButton->setText(action->text());
  d->insertActionInHistory(action);
  d->ModuleSearchLineEdit->setText("Search a module");
  emit moduleSelected(action->data().toString());
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectNextModule()
{
  CTK_D(qSlicerModuleSelectorToolBar);
  QAction* currentAction = d->lastSelectedAction();
  this->actionSelected(d->nextAction(currentAction));
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectPreviousModule()
{
  CTK_D(qSlicerModuleSelectorToolBar);
  QAction* currentAction = d->lastSelectedAction();
  this->actionSelected(d->previousAction(currentAction));
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::searchModule()
{
  CTK_D(qSlicerModuleSelectorToolBar);
  this->selectModuleByTitle(d->ModuleSearchLineEdit->text());
}
