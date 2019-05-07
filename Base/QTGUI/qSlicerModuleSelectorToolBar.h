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

#ifndef __qSlicerModuleSelectorToolBar_h
#define __qSlicerModuleSelectorToolBar_h

// Qt includes
#include <QToolBar>

// CTK includes
#include <ctkPimpl.h>
#include "qSlicerBaseQTGUIExport.h"

class ctkMenuComboBox;
class qSlicerModuleSelectorToolBarPrivate;
class qSlicerModuleManager;
class qSlicerModulesMenu;
class qSlicerAbstractCoreModule;

///
/// qSlicerModuleSelectorToolBar is a toolbar that can be added in your
/// application and will fire events when the QAction of modules are triggered
/// qSlicerModuleSelectorToolBar supports a tree hierarchy of modules (based on
/// module->categories() ), previous/next buttons to browse the history of
/// selected modules. (and a plain history button)
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModuleSelectorToolBar: public QToolBar
{
  Q_OBJECT
public:
  typedef QToolBar Superclass;
  Q_PROPERTY(QString selectedModule READ selectedModule WRITE selectModule NOTIFY moduleSelected)

  /// Constructor
  /// title is the name of the toolbar (can appear using right click on the
  /// toolbar area)
  qSlicerModuleSelectorToolBar(const QString& title, QWidget* parent = nullptr);
  qSlicerModuleSelectorToolBar(QWidget* parent = nullptr);
  ~qSlicerModuleSelectorToolBar() override;

  /// Returns a pointer to the modules menu used to populate the list of modules
  Q_INVOKABLE qSlicerModulesMenu* modulesMenu()const;

  /// Returns a pointer to the menu combobox used to display the modules menu.
  /// \sa modulesMenu()
  Q_INVOKABLE ctkMenuComboBox* modulesMenuComboBox() const;

  /// Returns the selected module name
  QString selectedModule() const;

public slots:
  /// Module manager contains all the loaded modules
  void setModuleManager(qSlicerModuleManager* moduleManager);

  /// Select a module by name. It looks for the module action and triggers it
  void selectModule(const QString& moduleName);

  void selectNextModule();
  void selectPreviousModule();

signals:
  /// The signal is fired every time a module is selected. The QAction of the
  /// module is triggered.
  /// TODO: directly connection modules QActions with the module manager
  void moduleSelected(const QString& name);

protected slots:
  void moduleRemoved(const QString& moduleName);

  void onModuleSelected(const QString& name);
  void actionSelected(QAction* action);

protected:
  QScopedPointer<qSlicerModuleSelectorToolBarPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModuleSelectorToolBar);
  Q_DISABLE_COPY(qSlicerModuleSelectorToolBar);
};

#endif
