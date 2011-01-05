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

#ifndef __qSlicerModuleSelectorToolBar_h
#define __qSlicerModuleSelectorToolBar_h

// Qt includes
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
#include "qSlicerBaseQTGUIExport.h"

class qSlicerModuleSelectorToolBarPrivate;
class qSlicerModuleManager;
class qSlicerAbstractCoreModule;

///
/// qSlicerModuleSelectorToolBar is a toolbar that can be added in your
/// application and will fired events when the QAction of modules are triggered
/// qSlicerModuleSelectorToolBar supports a tree hierarchy of modules (based on
/// module->category() ), previous/next buttons to browse the history of
/// selected modules. (and a plain history button)
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleSelectorToolBar: public QToolBar
{
  Q_OBJECT
public:
  typedef QToolBar Superclass;

  /// Constructor
  /// title is the name of the toolbar (can appear using right click on the
  /// toolbar area)
  qSlicerModuleSelectorToolBar(const QString& title, QWidget* parent = 0);
  qSlicerModuleSelectorToolBar(QWidget* parent = 0);
  virtual ~qSlicerModuleSelectorToolBar();

  ///
  /// Module manager contains all the loaded modules
  void setModuleManager(qSlicerModuleManager* moduleManager);
public slots:
  ///
  /// Select a module by title. It looks for the module action and triggers it
  void selectModuleByTitle(const QString& title);

  ///
  /// Select a module by name. It looks for the module action and triggers it
  void selectModule(const QString& moduleName);

  void selectNextModule();
  void selectPreviousModule();

signals:
  ///
  /// The signal is fired every time a module is selected. The QAction of the
  /// module is triggered.
  /// TODO: directly connection modules QActions with the module manager
  void moduleSelected(const QString& name);

protected slots:
  void moduleAdded(qSlicerAbstractCoreModule* );
  void moduleRemoved(qSlicerAbstractCoreModule* );

  void onModuleSelected(const QString& name);
  void actionSelected(QAction* action);
  void searchModule();

protected:
  QScopedPointer<qSlicerModuleSelectorToolBarPrivate> d_ptr;
  void modulesAdded(const QStringList& moduleNames);

private:
  Q_DECLARE_PRIVATE(qSlicerModuleSelectorToolBar);
  Q_DISABLE_COPY(qSlicerModuleSelectorToolBar);
};

#endif
