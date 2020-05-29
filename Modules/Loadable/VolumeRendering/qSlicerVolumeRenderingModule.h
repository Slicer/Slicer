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

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerVolumeRenderingModuleExport.h"

class qSlicerVolumeRenderingModulePrivate;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerVolumeRenderingModule(QObject *parent=nullptr);
  ~qSlicerVolumeRenderingModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help of the module
  QString helpText()const override;
  /// Acknowledgement for the module
  QString acknowledgementText()const override;
  /// Contributors of the module.
  QStringList contributors()const override;

  /// Return a custom icon for the module
  QIcon icon()const override;

  QStringList categories()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

protected:
  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerVolumeRenderingModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModule);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModule);

};

#endif
