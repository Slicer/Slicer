/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Yarmakovich, Isomics Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerVolumeRenderingModule_h
#define __qSlicerVolumeRenderingModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerVolumeRenderingModuleExport.h"

class qSlicerVolumeRenderingModulePrivate;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerVolumeRenderingModule(QObject *parent=0);
  virtual ~qSlicerVolumeRenderingModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help of the module
  virtual QString helpText()const;
  /// Acknowledgement for the module
  virtual QString acknowledgementText()const;
  /// Contributors of the module.
  virtual QStringList contributors()const;

  /// Return a custom icon for the module
  virtual QIcon icon()const;

  virtual QStringList categories()const;

  /// Specify editable node types
  virtual QStringList associatedNodeTypes()const;

protected:
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerVolumeRenderingModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModule);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModule);

};

#endif
