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

#ifndef __qSlicerSettingsModulesPanel_h
#define __qSlicerSettingsModulesPanel_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkSettingsPanel.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QSettings;
class qSlicerSettingsModulesPanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerSettingsModulesPanel
  : public ctkSettingsPanel
{
  Q_OBJECT

  /// This property holds the names of the modules to ignore at registration time.
  ///
  /// It corresponds to the names of the modules that are expected to always
  /// be ignored by saving and restoring them from the application settings.
  ///
  /// \sa qSlicerAbstractModuleFactoryManager::modulesToIgnore
  Q_PROPERTY(QStringList modulesToAlwaysIgnore
             READ modulesToAlwaysIgnore
             WRITE setModulesToAlwaysIgnore
             NOTIFY modulesToAlwaysIgnoreChanged)

public:
  /// Superclass typedef
  typedef ctkSettingsPanel Superclass;

  /// Constructor
  explicit qSlicerSettingsModulesPanel(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerSettingsModulesPanel() override;

  /// Get the \a modulesToAlwaysIgnore list.
  /// \sa setModulesToAlwaysIgnore(const QStringList& modulesNames)
  QStringList modulesToAlwaysIgnore()const;

public slots:

  /// Set the \a modulesToAlwaysIgnore list.
  ///
  /// If list is modified, the signal
  /// modulesToAlwaysIgnoreChanged(const QStringLists&) is emitted.
  ///
  /// \sa modulesToAlwaysIgnore()
  void setModulesToAlwaysIgnore(const QStringList& modulesNames);

protected slots:
  void onHomeModuleChanged(const QString& moduleName);
  void onTemporaryPathChanged(const QString& path);
  void onShowHiddenModulesChanged(bool);

  void onAdditionalModulePathsChanged();
  void onAddModulesAdditionalPathClicked();
  void onRemoveModulesAdditionalPathClicked();

signals:
  void modulesToAlwaysIgnoreChanged(const QStringList& modulesNames);

protected:
  QScopedPointer<qSlicerSettingsModulesPanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSettingsModulesPanel);
  Q_DISABLE_COPY(qSlicerSettingsModulesPanel);
};

#endif
