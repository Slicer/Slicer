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

#ifndef __qSlicerTractographyDisplayModule_h
#define __qSlicerTractographyDisplayModule_h

/// SlicerQT includes
#include "qSlicerLoadableModule.h"

#include "qSlicerTractographyDisplayModuleExport.h"

class qSlicerTractographyDisplayModulePrivate;

class Q_SLICER_QTMODULES_TRACTOGRAPHYDISPLAY_EXPORT qSlicerTractographyDisplayModule
  :public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);
public:
  typedef qSlicerLoadableModule Superclass;

  qSlicerTractographyDisplayModule(QObject *_parent = 0);

  ///
  /// Display name for the module
  virtual QString title()const {return "FiberBundles";}

  ///
  /// Help text of the module
  virtual QString helpText()const;

  ///
  /// Acknowledgement of the module
  virtual QString acknowledgementText()const;

protected:
  ///
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return a widget representation of the object
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();
};
#endif

