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

#ifndef __qSlicerEMSegmentModule_h
#define __qSlicerEMSegmentModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerEMSegmentModuleExport.h"

class qSlicerEMSegmentModulePrivate;

/// \ingroup Slicer_QtModules_EMSegment
class Q_SLICER_QTMODULES_EMSEGMENT_EXPORT qSlicerEMSegmentModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerEMSegmentModule(QObject *parent=0);
  virtual ~qSlicerEMSegmentModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  ///
  /// Help to use the module
  virtual QString helpText()const;

  ///
  /// Return the icon of the EMSegment module.
  virtual QIcon icon()const;

protected:
  ///
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerEMSegmentModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerEMSegmentModule);
  Q_DISABLE_COPY(qSlicerEMSegmentModule);
};

#endif
