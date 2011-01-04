/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerColorsModule_h
#define __qSlicerColorsModule_h

/// CTK includes
#include <ctkPimpl.h>

/// SlicerQt includes
#include "qSlicerCoreModule.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerColorsModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerColorsModule :
  public qSlicerCoreModule
{
  Q_OBJECT
public:
  typedef qSlicerCoreModule Superclass;
  qSlicerColorsModule(QObject *parent=0);
  virtual ~qSlicerColorsModule();

  virtual QIcon icon()const;
  qSlicerGetTitleMacro("Color");

  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

protected:
  /// Reimplemented to initialize the color logic
  virtual void setup();

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerColorsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerColorsModule);
  Q_DISABLE_COPY(qSlicerColorsModule);
};

#endif
