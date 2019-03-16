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

#ifndef __qSlicerAbstractModulePanel_h
#define __qSlicerAbstractModulePanel_h

#include "qSlicerWidget.h"
#include "qSlicerBaseQTGUIExport.h"

class qSlicerAbstractCoreModule;
class qSlicerAbstractModulePanelPrivate;
class qSlicerModuleManager;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerAbstractModulePanel: public qSlicerWidget
{
  Q_OBJECT
public:
  qSlicerAbstractModulePanel(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
  ~qSlicerAbstractModulePanel() override;

  void addModule(const QString& moduleName);
  void removeModule(const QString& moduleName);
  virtual void removeAllModules() = 0;

  Q_INVOKABLE void setModuleManager(qSlicerModuleManager* moduleManager);
  Q_INVOKABLE qSlicerModuleManager* moduleManager()const;

signals:
  void moduleAdded(const QString& moduleName);
  //void moduleAboutToBeRemoved(const QString& moduleName);
  void moduleRemoved(const QString& moduleName);

protected:
  QScopedPointer<qSlicerAbstractModulePanelPrivate> d_ptr;

  virtual void addModule(qSlicerAbstractCoreModule* module) = 0;
  virtual void removeModule(qSlicerAbstractCoreModule* module) = 0;

private:
  Q_DECLARE_PRIVATE(qSlicerAbstractModulePanel);
  Q_DISABLE_COPY(qSlicerAbstractModulePanel);
};

#endif
