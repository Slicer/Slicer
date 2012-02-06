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

// SlicerQt includes
#include "qSlicerAbstractModulePanel.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerModulePanelPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerModulePanel
  : public qSlicerAbstractModulePanel
{
  Q_OBJECT
public:
  typedef qSlicerAbstractModulePanel Superclass;
  qSlicerModulePanel(QWidget* parent = 0, Qt::WindowFlags f = 0);
  virtual ~qSlicerModulePanel();

  virtual void removeAllModules();
  qSlicerAbstractCoreModule* currentModule()const;
  QString currentModuleName()const;

  virtual bool eventFilter(QObject* watchedModule, QEvent* event);
  virtual QSize minimumSizeHint()const;

public slots:
  void setModule(const QString& moduleName);

protected:
  virtual void addModule(qSlicerAbstractCoreModule* module);
  virtual void removeModule(qSlicerAbstractCoreModule* module);
  void setModule(qSlicerAbstractCoreModule* module);

protected:
  QScopedPointer<qSlicerModulePanelPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerModulePanel);
  Q_DISABLE_COPY(qSlicerModulePanel);
};

#endif
