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

#ifndef __qSlicerModulesMenu_h
#define __qSlicerModulesMenu_h

// Qt includes
#include <QMenu>

// CTK includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerModulesMenuPrivate;

///
/// qSlicerModulesMenu supports a tree hierarchy of modules (based on
/// module->category() )
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulesMenu: public QMenu
{
  Q_OBJECT
public:
  typedef QMenu Superclass;

  /// Constructor
  /// title is the name of the menu (can appear using right click on the
  /// toolbar area)
  qSlicerModulesMenu(const QString& title, QWidget* parent = 0);
  qSlicerModulesMenu(QWidget* parent = 0);
  virtual ~qSlicerModulesMenu();

  ///
  QAction* moduleAction(const QString& moduleName)const;

  ///
  /// Add a module by name. The module action will be inserted
  /// based on its category.
  void addModule(const QString& moduleName);

  ///
  /// Add a list of module available for selection
  inline void addModules(const QStringList& moduleNames);

  ///
  /// Remove the module from the list of available module
  void removeModule(const QString& moduleName);

public slots:
  ///
  /// Select a module by title. It looks for the module action and triggers it
  void selectModuleByTitle(const QString& title);

  ///
  /// Select a module by name. It looks for the module action and triggers it
  void selectModule(const QString& moduleName);

signals:
  ///
  /// The signal is fired every time a module is selected. The QAction of the
  /// module is triggered.
  void moduleSelected(const QString& name);

protected slots:
  void onActionTriggered();
  void actionSelected(QAction* action);

protected:
  QScopedPointer<qSlicerModulesMenuPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModulesMenu);
  Q_DISABLE_COPY(qSlicerModulesMenu);
};

//---------------------------------------------------------------------------
void qSlicerModulesMenu::addModules(const QStringList& moduleNames)
{
  foreach(const QString& moduleName, moduleNames)
    {
    this->addModule(moduleName);
    }
}

#endif
