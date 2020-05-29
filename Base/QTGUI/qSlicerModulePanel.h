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

#ifndef __qSlicerModulePanel_h
#define __qSlicerModulePanel_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerAbstractModulePanel.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerModulePanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulePanel
  : public qSlicerAbstractModulePanel
{
  Q_OBJECT

  /// This property controls whether the help and acknowledgment
  /// section is visible or not.
  /// If the property is set to \a true and the current module doesn't have
  /// any help text, the section will remain hidden.
  /// If the property has already been set to \a true and a module
  /// with a help text is set, the section will be visible.
  /// \sa isHelpAndAcknowledgmentVisible()
  /// \sa setHelpAndAcknowledgmentVisible()
  /// \sa qSlicerAbstractCoreModule::helpText(), setModule()
  Q_PROPERTY(bool helpAndAcknowledgmentVisible READ isHelpAndAcknowledgmentVisible WRITE setHelpAndAcknowledgmentVisible)

public:
  typedef qSlicerAbstractModulePanel Superclass;
  qSlicerModulePanel(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~qSlicerModulePanel() override;

  /// Get the helpAndAcknowledgmentVisible property value.
  /// \sa helpAndAcknowledgmentVisible, isHelpAndAcknowledgmentVisible()
  void setHelpAndAcknowledgmentVisible(bool value);

  /// Set the canShowHelpAndAcknowledgment property value.
  /// \sa helpAndAcknowledgmentVisible, setHelpAndAcknowledgmentVisible()
  bool isHelpAndAcknowledgmentVisible()const;

  void removeAllModules() override;
  qSlicerAbstractCoreModule* currentModule()const;
  QString currentModuleName()const;

  bool eventFilter(QObject* watchedModule, QEvent* event) override;
  QSize minimumSizeHint()const override;

public slots:
  void setModule(const QString& moduleName);

protected:
  void addModule(qSlicerAbstractCoreModule* module) override;
  void removeModule(qSlicerAbstractCoreModule* module) override;
  void setModule(qSlicerAbstractCoreModule* module);

protected:
  QScopedPointer<qSlicerModulePanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModulePanel);
  Q_DISABLE_COPY(qSlicerModulePanel);
};

#endif
