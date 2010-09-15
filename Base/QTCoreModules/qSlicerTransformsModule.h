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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerTransformsModule_h
#define __qSlicerTransformsModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerCoreModule.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTransformsModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerTransformsModule :
  public qSlicerCoreModule
{
  Q_OBJECT
public:

  typedef qSlicerCoreModule Superclass;
  qSlicerTransformsModule(QObject *parent=0);
  virtual ~qSlicerTransformsModule();

  ///
  /// Icon of the transform module
  virtual QIcon icon()const;

  ///
  /// Display name for the module
  qSlicerGetTitleMacro("Transforms");

  ///
  /// Help text of the module
  virtual QString helpText()const;

  ///
  /// Acknowledgement of the module
  virtual QString acknowledgementText()const;

protected:
  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

  QScopedPointer<qSlicerTransformsModulePrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qSlicerTransformsModule);
  Q_DISABLE_COPY(qSlicerTransformsModule);
};

#endif
