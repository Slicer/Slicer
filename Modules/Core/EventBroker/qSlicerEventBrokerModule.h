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

#ifndef __qSlicerEventBrokerModule_h
#define __qSlicerEventBrokerModule_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerCoreModule.h"

#include "qSlicerModulesCoreExport.h"

class qSlicerEventBrokerModulePrivate;

class Q_SLICER_MODULES_CORE_EXPORT qSlicerEventBrokerModule :
  public qSlicerCoreModule
{
  Q_OBJECT
public:

  typedef qSlicerCoreModule Superclass;
  qSlicerEventBrokerModule(QObject *parent=nullptr);
  ~qSlicerEventBrokerModule() override;

  QStringList categories()const override;

  /// Display name for the module
  qSlicerGetTitleMacro("Event Broker");

  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

protected:
  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

  QScopedPointer<qSlicerEventBrokerModulePrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qSlicerEventBrokerModule);
  Q_DISABLE_COPY(qSlicerEventBrokerModule);
};

#endif
