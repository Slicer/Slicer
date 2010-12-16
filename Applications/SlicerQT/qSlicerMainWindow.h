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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerMainWindow_h
#define __qSlicerMainWindow_h 

// Qt includes
#include <QMainWindow>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

#include "qSlicerQTExport.h"

class qSlicerAbstractCoreModule;
class qSlicerModulePanel;
class qSlicerModuleSelectorToolBar;
class qSlicerMainWindowCore;
class qSlicerMainWindowPrivate;

class Q_SLICERQT_EXPORT qSlicerMainWindow : public QMainWindow
{
  Q_OBJECT
  QVTK_OBJECT
public:

  typedef QMainWindow Superclass;
  qSlicerMainWindow(QWidget *parent=0);
  virtual ~qSlicerMainWindow();

  /// Return the main window core.
  qSlicerMainWindowCore* core()const;

  /// Return the module selector
  Q_INVOKABLE qSlicerModuleSelectorToolBar* moduleSelector()const;

protected slots:
  void onModuleLoaded(qSlicerAbstractCoreModule* module);
  void onModuleAboutToBeUnloaded(qSlicerAbstractCoreModule* module);
  void onViewExtensionManagerActionTriggered();
  void onViewApplicationSettingsActionTriggered();

  void onMRMLSceneModified(vtkObject*);
protected:

  // Description:
  // Connect MainWindow action with slots defined in MainWindowCore
  void setupMenuActions();

  virtual void closeEvent(QCloseEvent *event);

protected:
  QScopedPointer<qSlicerMainWindowPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMainWindow);
  Q_DISABLE_COPY(qSlicerMainWindow);
};

#endif
