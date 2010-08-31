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
