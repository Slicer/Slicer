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

#ifndef __qSlicerCamerasModule_h
#define __qSlicerCamerasModule_h

/// CTK includes
#include <ctkPimpl.h>

/// SlicerQt includes
#include "qSlicerCoreModule.h"

#include "qSlicerModulesCoreExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerCamerasModulePrivate;

class Q_SLICER_MODULES_CORE_EXPORT qSlicerCamerasModule :
  public qSlicerCoreModule
{
  Q_OBJECT
public:
  typedef qSlicerCoreModule Superclass;
  qSlicerCamerasModule(QObject *parent=0);
  virtual ~qSlicerCamerasModule();

  virtual QStringList categories()const;
  virtual QIcon icon()const;
  qSlicerGetTitleMacro("Cameras");

  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QStringList contributors()const;

protected:

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerCamerasModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCamerasModule);
  Q_DISABLE_COPY(qSlicerCamerasModule);
};

#endif
