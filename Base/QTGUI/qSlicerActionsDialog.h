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

#ifndef __qSlicerActionsDialog_h
#define __qSlicerActionsDialog_h

// Qt includes
#include <QDialog>
#include <QScopedPointer>

// Slicer includes
#include "qSlicerBaseQTGUIExport.h"
class qSlicerActionsDialogPrivate;

//------------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerActionsDialog: public QDialog
{
  Q_OBJECT
public:
  typedef QDialog Superclass;
  qSlicerActionsDialog(QWidget* parentWidget =nullptr);
  ~qSlicerActionsDialog() override;

  void addAction(QAction* action, const QString& group=QString());
  void addActions(const QList<QAction*>& actions, const QString& group=QString());
  void clear();

  void setActionsWithNoShortcutVisible(bool visible);
  void setMenuActionsVisible(bool visible);

protected:
  QScopedPointer<qSlicerActionsDialogPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerActionsDialog);
  Q_DISABLE_COPY(qSlicerActionsDialog);
};

#endif
