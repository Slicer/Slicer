#include "qSlicerActionsDialog.h"

//------------------------------------------------------------------------------
qSlicerActionsDialog::qSlicerActionsDialog(QWidget* parentWidget)
  :QDialog(parentWidget)
{
  this->setupUi(this);
}

//------------------------------------------------------------------------------
qSlicerActionsDialog::~qSlicerActionsDialog()
{
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::addAction(QAction* action, const QString& group)
{
  this->ActionsWidget->addAction(action, group);
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::addActions(const QList<QAction*>& actions,
                                      const QString& group)
{
  this->ActionsWidget->addActions(actions, group);
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::clear()
{
  this->ActionsWidget->clear();
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::setActionsWithNoShortcutVisible(bool visible)
{
  this->ActionsWidget->setActionsWithNoShortcutVisible(visible);
}

//------------------------------------------------------------------------------
void qSlicerActionsDialog::setMenuActionsVisible(bool visible)
{
  this->ActionsWidget->setMenuActionsVisible(visible);
}
