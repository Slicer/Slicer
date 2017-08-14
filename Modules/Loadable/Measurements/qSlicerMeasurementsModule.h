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

#ifndef __qSlicerMeasurementsModule_h
#define __qSlicerMeasurementsModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerMeasurementsModuleExport.h"

class qSlicerMeasurementsModulePrivate;

/// \ingroup Slicer_QtModules_Measurements
class Q_SLICER_QTMODULES_MEASUREMENTS_EXPORT qSlicerMeasurementsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerMeasurementsModule(QObject *parent=0);
  virtual ~qSlicerMeasurementsModule();

  virtual QIcon icon()const;
  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

protected:
  QScopedPointer<qSlicerMeasurementsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMeasurementsModule);
  Q_DISABLE_COPY(qSlicerMeasurementsModule);
};

#endif
