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

#ifndef __qSlicerModulesListView_h
#define __qSlicerModulesListView_h

// Qt includes
#include <QListView>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerModulesListViewPrivate;
class qSlicerModuleFactoryFilterModel;
class qSlicerAbstractModuleFactoryManager;
class QStandardItem;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulesListView : public QListView
{
  Q_OBJECT
  /// False by default
  Q_PROPERTY(bool checkBoxVisible READ isCheckBoxVisible WRITE setCheckBoxVisible )
  Q_PROPERTY(QStringList modules READ modules)
  Q_PROPERTY(QStringList checkedModules READ checkedModules
             WRITE setCheckedModules NOTIFY checkedModulesChanged
             DESIGNABLE false)
  Q_PROPERTY(QStringList uncheckedModules READ uncheckedModules
             WRITE setUncheckedModules NOTIFY uncheckedModulesChanged
             DESIGNABLE false)
public:
  /// Superclass typedef
  typedef QListView Superclass;

  /// Constructor
  explicit qSlicerModulesListView(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerModulesListView() override;

  /// Get the module factory manager that contains the list of modules
  /// and modules to ignore
  Q_INVOKABLE qSlicerAbstractModuleFactoryManager* factoryManager()const;

  Q_INVOKABLE qSlicerModuleFactoryFilterModel* filterModel()const;

  /// Return the list of all loaded, ignored and toIgnore modules.
  QStringList modules()const;

  bool isCheckBoxVisible()const;
  /// Return the list of checked modules. It is the list of all the modules
  /// to load at startup.
  QStringList checkedModules()const;

  /// Return the list of unchecked modules. Please note it is the same
  /// than the qSlicerModuleFactoryManager::modulesToIgnore property.
  QStringList uncheckedModules()const;

public slots:
  /// Set the module factory manager that contains the list of modules
  /// and modules to ignore
  void setFactoryManager(qSlicerAbstractModuleFactoryManager* manager);

  void setCheckBoxVisible(bool show);

  /// Check the modules in the \a moduleNames list. Uncheck the modules
  /// not in the \a moduleNames list.
  void setCheckedModules(const QStringList& moduleNames);

  /// Uncheck the modules in the \a moduleNames list. Check the modules
  /// not in the \a moduleNames list.
  void setUncheckedModules(const QStringList& moduleNames);

  void hideSelectedModules();
  void moveLeftSelectedModules();
  void moveRightSelectedModules();
  void moveSelectedModules(int offset);

  void scrollToSelectedModules();
signals:
  /// This signal is emitted when a module is checked
  void checkedModulesChanged(const QStringList&);
  /// This signal is emitted when a module is unchecked
  void uncheckedModulesChanged(const QStringList&);

protected slots:
  void addModules(const QStringList& moduleNames);
  void addModule(const QString& moduleName);
  void updateModules();
  void updateModules(const QStringList& moduleName);
  void updateModule(const QString& moduleName);
  void onItemChanged(QStandardItem* item);

protected:
  QScopedPointer<qSlicerModulesListViewPrivate> d_ptr;

  void changeEvent(QEvent* e) override;
  void keyPressEvent(QKeyEvent * event) override;

private:
  Q_DECLARE_PRIVATE(qSlicerModulesListView);
  Q_DISABLE_COPY(qSlicerModulesListView);
};

#endif

