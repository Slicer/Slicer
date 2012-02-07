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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerViewControllersModule_h
#define __qSlicerViewControllersModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerViewControllersModuleExport.h"

class qSlicerViewControllersModulePrivate;

class Q_SLICER_QTMODULES_VIEWCONTROLLERS_EXPORT qSlicerViewControllersModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerViewControllersModule(QObject *parent=0);
  virtual ~qSlicerViewControllersModule();

  qSlicerGetTitleMacro("View Controllers");

  virtual QString acknowledgementText()const;
  virtual QStringList categories()const;
  virtual QStringList contributors()const;

protected:
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerViewControllersModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerViewControllersModule);
  Q_DISABLE_COPY(qSlicerViewControllersModule);
};

#endif
