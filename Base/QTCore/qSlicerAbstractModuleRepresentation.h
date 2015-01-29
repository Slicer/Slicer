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

#ifndef __qSlicerAbstractModuleRepresentation_h
#define __qSlicerAbstractModuleRepresentation_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerObject.h"

#include "qSlicerBaseQTCoreExport.h"

/// class vtkSlicerApplicationLogic;
class vtkMRMLAbstractLogic;
class qSlicerAbstractCoreModule;
class QAction;
class qSlicerAbstractModuleRepresentationPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerAbstractModuleRepresentation : virtual public qSlicerObject
{
public:

  typedef qSlicerObject Superclass;
  qSlicerAbstractModuleRepresentation();
  virtual ~qSlicerAbstractModuleRepresentation();

  /// Set/Get module name
  QString moduleName()const;

  /// Returns the module the representation belongs to.
  /// The module is set right before setup() is called.
  qSlicerAbstractCoreModule* module()const;

protected:
  /// All initialization code (typically setupUi()) must be done in setup()
  /// The module and the logic are accessible.
  virtual void setup() = 0;

  /// Return the logic of the module
  /// The logic is available (not null) when setup() is called. So you can't
  /// access it in the constructor of your module widget.
  vtkMRMLAbstractLogic* logic() const;

protected:
  QScopedPointer<qSlicerAbstractModuleRepresentationPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAbstractModuleRepresentation);
  Q_DISABLE_COPY(qSlicerAbstractModuleRepresentation);

  /// Give access to qSlicerAbstractModule to the method
  /// qSlicerAbstractModuleRepresentation::setModule
  friend class qSlicerAbstractCoreModule;
  void setModule(qSlicerAbstractCoreModule* module);
};

#endif
