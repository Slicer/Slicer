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

#ifndef __qSlicerTractographyDisplayModule_h
#define __qSlicerTractographyDisplayModule_h

// CTK includes
#include <ctkPimpl.h>

/// SlicerQT includes
#include "qSlicerLoadableModule.h"

#include "qSlicerTractographyDisplayModuleExport.h"

class qSlicerTractographyDisplayModulePrivate;

/// \ingroup Slicer_QtModules_TractographyDisplay
class Q_SLICER_QTMODULES_TRACTOGRAPHYDISPLAY_EXPORT qSlicerTractographyDisplayModule
  :public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);
public:
  typedef qSlicerLoadableModule Superclass;

  qSlicerTractographyDisplayModule(QObject *_parent = 0);

  /// Categories of the module
  virtual QStringList categories() const;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QString contributor()const;

protected:
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return a widget representation of the object
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();
};
#endif

