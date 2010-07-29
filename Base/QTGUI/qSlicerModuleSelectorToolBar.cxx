
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

  QAction* action(const QVariant& actionData, QMenu* parentMenu)const;
  QAction* action(const QString& text, QMenu* parentMenu)const;
  QAction* lastSelectedAction()const;
  QMenu*   actionMenu(QAction* action, QMenu* parentMenu)const;

  void insertActionOnTop(QAction* action, QMenu* menu);

  QMenu*       AllModulesMenu;
  QMenu*       ModulesMenu;
  QPushButton* ModulesButton;
  QMenu*       HistoryMenu;
  QToolButton* HistoryButton;
  QToolButton* PreviousButton;
  QMenu*       PreviousHistoryMenu;
  QToolButton* NextButton;
  QMenu*       NextHistoryMenu;
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
  this->PreviousButton = 0;
  this->PreviousHistoryMenu = 0;
  this->NextButton = 0;
  this->NextHistoryMenu = 0;
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
  p->addWidget(new QLabel(QObject::tr("Modules:"), p));

  // Modules pushbutton
  this->ModulesMenu = new QMenu(QObject::tr("Modules"),p);
  this->ModulesButton = new QPushButton(p);
  this->ModulesButton->setMenu(this->ModulesMenu);
  QStyleOptionButton opt;
  this->ModulesButton->setFixedWidth(
    this->ModulesButton->style()->sizeFromContents(
      QStyle::CT_PushButton, &opt,
      this->ModulesButton->fontMetrics().size(
        Qt::TextShowMnemonic, "XXXXXXXXXXXXXXXXXXXXXXXXXX"),this->ModulesButton).width());
  p->addWidget(this->ModulesButton);

  // Previous button
  this->PreviousHistoryMenu = new QMenu(p);
  this->PreviousButton = new QToolButton(p);
  this->PreviousButton->setIcon(previousIcon);
  this->PreviousButton->setText(QObject::tr("Previous"));
  this->PreviousButton->setMenu(this->PreviousHistoryMenu);
  this->PreviousButton->setArrowType(Qt::NoArrow);
  // selectPreviousModule is called only if the toolbutton is clicked not if an
  // action in the history is triggered
  QObject::connect(this->PreviousButton, SIGNAL(clicked(bool)),
                   p, SLOT(selectPreviousModule()));
  p->addWidget(this->PreviousButton);
  this->PreviousButton->setEnabled(this->PreviousHistoryMenu->actions().size() > 0);

  // Next button
  this->NextHistoryMenu = new QMenu(p);
  this->NextButton = new QToolButton(p);
  this->NextButton->setIcon(nextIcon);
  this->NextButton->setText(QObject::tr("Next"));
  this->NextButton->setMenu(this->NextHistoryMenu);
  this->NextButton->setArrowType(Qt::NoArrow);
  // selectNextModule is called only if the toolbutton is clicked not if an
  // action in the history is triggered
  QObject::connect(this->NextButton, SIGNAL(clicked(bool)),
                   p, SLOT(selectNextModule()));
  p->addWidget(this->NextButton);
  this->NextButton->setEnabled(this->NextHistoryMenu->actions().size() > 0);

  // History
  this->HistoryMenu = new QMenu(QObject::tr("Modules history"), p);
  this->HistoryButton = new QToolButton;
  this->HistoryButton->setIcon(historyIcon);
  this->HistoryButton->setMenu(this->HistoryMenu);
  this->HistoryButton->setPopupMode(QToolButton::InstantPopup);
  p->addWidget(this->HistoryButton);

  // Search
  this->ModuleSearchLineEdit = new QLineEdit(p);
  // TODO clear the search text when the line edit gets the focus
  this->ModuleSearchLineEdit->setText(QObject::tr("Search a module"));
  this->ModuleSearchCompleter = new QCompleter(QStringList());
  this->ModuleSearchCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  this->ModuleSearchLineEdit->setCompleter(this->ModuleSearchCompleter);
  QObject::connect(this->ModuleSearchLineEdit, SIGNAL(textChanged(const QString&)),
                   p, SLOT(selectModuleByTitle(const QString&)));
  p->addWidget(this->ModuleSearchLineEdit);

  this->addDefaultCategories();
}

void qSlicerModuleSelectorToolBarPrivate::addDefaultCategories()
{
  this->AllModulesMenu = this->ModulesMenu->addMenu(QObject::tr("All Modules"));
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
QAction* qSlicerModuleSelectorToolBarPrivate::lastSelectedAction()const
{
  QList<QAction*> actions = this->HistoryMenu->actions();
  return actions.size() ? actions[0] : 0;
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
void qSlicerModuleSelectorToolBarPrivate::insertActionOnTop(QAction* action, QMenu* menu)
{
  menu->removeAction(action);
  QAction* before = menu->actions().isEmpty() ? 0 : menu->actions().first();
  menu->insertAction(before, action);
  QList<QAction*> actions = menu->actions();
  for (int i = 8; i < actions.size(); ++i)
    {
    menu->removeAction(actions[i]);
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
  QObject::connect(moduleAction, SIGNAL(triggered(bool)),
                   this, SLOT(onActionTriggered()));

  QMenu* menu = d->menu(d->ModulesMenu, module->category().split('.'));
  d->addModuleAction(menu, moduleAction);
  // Add in "All Modules" as well
  d->addModuleAction(d->AllModulesMenu, moduleAction);

  // here we assume the completion model is not automatically sorted
  int actionCount = d->ModuleSearchCompleter->model()->rowCount();
  d->ModuleSearchCompleter->model()->insertRow(actionCount);
  QModelIndex index = d->ModuleSearchCompleter->model()->index(actionCount -1, 0);
  d->ModuleSearchCompleter->model()->setData(index, module->title());
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::removeModule(const QString& moduleName)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  // removing a module consists in retrieving the unique action of the module
  // and removing it from all the possible menus
  QAction* moduleAction = d->action(QVariant(moduleName), d->ModulesMenu);
  QMenu* menu = d->actionMenu(moduleAction, d->ModulesMenu);
  menu->removeAction(moduleAction);
  d->AllModulesMenu->removeAction(moduleAction);
  d->HistoryMenu->removeAction(moduleAction);
  d->PreviousHistoryMenu->removeAction(moduleAction);
  d->NextHistoryMenu->removeAction(moduleAction);
  // TBD: what if the module is the current module ?
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectModule(const QString& moduleName)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  // It's faster to look for the action in the AllModulesMenu (no need to
  // do a recursive search
  QAction* moduleAction = d->action(QVariant(moduleName), d->AllModulesMenu);
  if (moduleAction)
    {
    // triggering the action will eventually call actionSelected();
    moduleAction->trigger();
    }
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectModuleByTitle(const QString& title)
{
  CTK_D(qSlicerModuleSelectorToolBar);
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
void qSlicerModuleSelectorToolBar::onActionTriggered()
{
  // we know for sure that the sender is the triggered QAction
  this->actionSelected(qobject_cast<QAction*>(this->sender()));
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::actionSelected(QAction* action)
{
  CTK_D(qSlicerModuleSelectorToolBar);
  QAction* lastAction = d->lastSelectedAction();
  if (action == lastAction)
    {
    return;
    }
  QList<QAction*> previousActions = d->PreviousHistoryMenu->actions();
  QList<QAction*> nextActions = d->NextHistoryMenu->actions();
  int actionIndexInPreviousMenu = previousActions.indexOf(action);
  int actionIndexInNextMenu = nextActions.indexOf(action);
  if ( actionIndexInNextMenu >= 0)
    {
    previousActions.push_front(lastAction);
    for (int i = 0; i < actionIndexInNextMenu ; ++i)
      {
      previousActions.push_front(nextActions.takeFirst());
      }
    Q_ASSERT(nextActions[0] == action);
    nextActions.removeFirst();
    }
  else if ( actionIndexInPreviousMenu >= 0)
    {
    nextActions.push_front(lastAction);
    for (int i = 0; i < actionIndexInPreviousMenu  ; ++i)
      {
      nextActions.push_front(previousActions.takeFirst());
      }
    Q_ASSERT(previousActions[0] == action);
    previousActions.removeFirst();
    }
  else
    {
    previousActions.push_front(lastAction);
    nextActions.clear();
    }
  // don't keep more than X history
  previousActions = previousActions.mid(0, 8);
  nextActions = nextActions.mid(0, 8);

  d->PreviousHistoryMenu->clear();
  d->PreviousHistoryMenu->addActions(previousActions);
  d->NextHistoryMenu->clear();
  d->NextHistoryMenu->addActions(nextActions);

  d->ModulesButton->setIcon(action->icon());
  d->ModulesButton->setText(action->text());
  d->PreviousButton->setEnabled(d->PreviousHistoryMenu->actions().size());
  d->NextButton->setEnabled(d->NextHistoryMenu->actions().size());

  d->insertActionOnTop(action, d->HistoryMenu);
  d->ModuleSearchLineEdit->setText("Search a module");
  emit moduleSelected(action->data().toString());
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectNextModule()
{
  CTK_D(qSlicerModuleSelectorToolBar);
  // selectNextModule() is not called when an action from the next history menu
  // is triggered. selectNextModule() is called only if the next toolbutton is
  // clicked.
  QList<QAction*> actions = d->NextHistoryMenu->actions();
  QAction* nextAction = actions.size() ? actions.first() : 0;
  if (nextAction)
    {
    // triggering the action will eventually call actionSelected()
    nextAction->trigger();
    }
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::selectPreviousModule()
{
  CTK_D(qSlicerModuleSelectorToolBar);
  // selectPreviousModule() is not called when an action from the Previous
  // history menu is triggered. selectPreviousModule() is called only if the
  // previous toolbutton is clicked.
  QList<QAction*> actions = d->PreviousHistoryMenu->actions();
  QAction* previousAction = actions.size() ? actions.first() : 0;
  if (previousAction)
    {
    // triggering the action will eventually call actionSelected()
    previousAction->trigger();
    }
}

//---------------------------------------------------------------------------
void qSlicerModuleSelectorToolBar::searchModule()
{
  CTK_D(qSlicerModuleSelectorToolBar);
  this->selectModuleByTitle(d->ModuleSearchLineEdit->text());
}
