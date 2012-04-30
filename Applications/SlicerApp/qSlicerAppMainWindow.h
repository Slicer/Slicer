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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerAppMainWindow_h
#define __qSlicerAppMainWindow_h

// Qt includes
#include <QMainWindow>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qSlicerAppExport.h"

class qSlicerAbstractCoreModule;
class qSlicerModulePanel;
class qSlicerModuleSelectorToolBar;
class qSlicerAppMainWindowCore;
class qSlicerAppMainWindowPrivate;

class Q_SLICER_APP_EXPORT qSlicerAppMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:

  typedef QMainWindow Superclass;
  qSlicerAppMainWindow(QWidget *parent=0);
  virtual ~qSlicerAppMainWindow();

  /// Return the main window core.
  qSlicerAppMainWindowCore* core()const;

  /// Return the module selector
  Q_INVOKABLE qSlicerModuleSelectorToolBar* moduleSelector()const;

public slots:
  void setHomeModuleCurrent();
  void restoreToolbars();

protected slots:
  void onModuleLoaded(const QString& moduleName);
  void onModuleAboutToBeUnloaded(const QString& moduleName);
  void onEditApplicationSettingsActionTriggered();
  void onViewExtensionManagerActionTriggered();

  void onMRMLSceneModified(vtkObject*);
  void onLayoutActionTriggered(QAction* action);
  void onLayoutCompareActionTriggered(QAction* action);
  void onLayoutCompareWidescreenActionTriggered(QAction* action);
  void onLayoutCompareGridActionTriggered(QAction* action);
  void onLayoutChanged(int);
  void loadDICOMActionTriggered();

protected:

  /// Connect MainWindow action with slots defined in MainWindowCore
  void setupMenuActions();

  /// Open a popup to warn the user Slicer is not for clinical use.
  void disclaimer();

  /// Forward the dragEnterEvent to the IOManager which will
  /// decide if it could accept a drag/drop or not.
  void dragEnterEvent(QDragEnterEvent *event);

  /// Forward the dropEvent to the IOManager.
  void dropEvent(QDropEvent *event);

  /// Reimplemented to catch show/hide events
  bool eventFilter(QObject* object, QEvent* event);

  virtual void closeEvent(QCloseEvent *event);
  virtual void showEvent(QShowEvent *event);

protected:
  QScopedPointer<qSlicerAppMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAppMainWindow);
  Q_DISABLE_COPY(qSlicerAppMainWindow);
};

#endif
