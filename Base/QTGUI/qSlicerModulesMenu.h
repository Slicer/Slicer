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

#ifndef __qSlicerModulesMenu_h
#define __qSlicerModulesMenu_h

// Qt includes
#include <QMenu>

// CTK includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerAbstractCoreModule;
class qSlicerModulesMenuPrivate;
class qSlicerModuleManager;

///
/// qSlicerModulesMenu supports a tree hierarchy of modules (based on
/// module->category() )
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulesMenu: public QMenu
{
  Q_OBJECT
  Q_PROPERTY(QString currentModule READ currentModule WRITE setCurrentModule NOTIFY currentModuleChanged)
  /// By default (duplicateActions = false), multiple instances of
  /// qSlicerModulesMenu share the same QActions. When a module QAction is
  /// fired from a menu, all the qSlicerModulesMenu would make it the current
  /// module. When duplicateActions is true, the QActions populating the menu are
  /// duplicates from the original module QAction. That way the qSlicerModulesMenu
  /// behaves independently from the other qSlicerModulesMenus.
  /// Note: this property should be set before modules are added (addModule()).
  Q_PROPERTY(bool duplicateActions READ duplicateActions WRITE setDuplicateActions)

  /// By default (showHiddenModules == false), modules with the hidden property
  /// set to true are not shown. If showHiddenModules is true, all the modules
  /// are visible.
  /// Note: this property should be set before modules are added (addModule),
  /// changing its value won't change the visibility of the current volumes but
  /// only the future added modules
  Q_PROPERTY(bool showHiddenModules READ showHiddenModules WRITE setShowHiddenModules)

  Q_PROPERTY(QStringList topLevelCategoryOrder READ topLevelCategoryOrder WRITE setTopLevelCategoryOrder)
public:
  typedef QMenu Superclass;

  /// Constructor
  /// title is the name of the menu (can appear using right click on the
  /// toolbar area)
  qSlicerModulesMenu(const QString& title, QWidget* parent = nullptr);
  qSlicerModulesMenu(QWidget* parent = nullptr);
  ~qSlicerModulesMenu() override;

  ///
  Q_INVOKABLE QAction* moduleAction(const QString& moduleName)const;

  /// Add a list of module available for selection.
  inline void addModules(const QStringList& moduleNames);

  /// Add a list of module available for selection.
  inline void removeModules(const QStringList& moduleNames);

  /// Return the last selected module name.
  QString currentModule()const;

  /// Set the module manager to retrieve the modules from.
  void setModuleManager(qSlicerModuleManager* moduleManager);
  qSlicerModuleManager* moduleManager()const;

  void setDuplicateActions(bool duplicate);
  bool duplicateActions()const;

  /// If true, modules with the hidden property set to true are still visible
  /// in the module.
  void setShowHiddenModules(bool show);
  bool showHiddenModules()const;

  void setTopLevelCategoryOrder(const QStringList& categories);
  QStringList topLevelCategoryOrder()const;

  /// Remove a top-level category or sub-category. Return true if it was found and removed.
  ///
  /// Sub-category can be specified using a "dot" separator (i.e. "CategoryName.SubCategoryName")
  ///
  /// \sa removeModule()
  Q_INVOKABLE bool removeCategory(const QString& categoryName);

public slots:
  /// Add a module by name into the menu.
  ///
  /// The category property of the module is used to assign a submenu to the
  /// module action. If a module is hidden and showHiddenModules is false
  /// (default), the module is ignored and not added into the list
  ///
  /// \sa qSlicerAbstractCoreModule::category()
  /// \sa qSlicerAbstractCoreModule::action()
  /// \sa qSlicerAbstractCoreModule::isHidden()
  void addModule(const QString& moduleName);

  /// Remove the module from the menu.
  ///
  /// Return true if the module was found and removed.
  ///
  /// By default, matching module entries are removed from the top-level category,
  /// the custom and pre-defined categories.
  ///
  /// \a removeFromAllModules to false allows to change this.
  bool removeModule(const QString& moduleName);

  /// Select a module by title. It looks for the module action and triggers it.
  /// \sa setCurrentModule()
  void setCurrentModuleByTitle(const QString& title);

  /// Select a module by name. It looks for the module action and triggers it.
  /// \sa setCurrentModuleByTitle()
  void setCurrentModule(const QString& moduleName);

  /// Add a module into the menu.
  void addModule(qSlicerAbstractCoreModule*);

  /// Remove the module from the menu.
  ///
  /// By default, matching module entries are removed from the top-level category,
  /// the custom and pre-defined categories as well as the "All Modules" special
  /// category. Setting \a removeFromAllModules to false allows to change this.
  ///
  /// Return true if the module was found and removed.
  bool removeModule(qSlicerAbstractCoreModule*);

signals:
  /// The signal is fired every time a module is selected. The QAction of the
  /// module is triggered.
  void currentModuleChanged(const QString& name);

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

//---------------------------------------------------------------------------
void qSlicerModulesMenu::removeModules(const QStringList& moduleNames)
{
  foreach(const QString& moduleName, moduleNames)
    {
    this->removeModule(moduleName);
    }
}

#endif
