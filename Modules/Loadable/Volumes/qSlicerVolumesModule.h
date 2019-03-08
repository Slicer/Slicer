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

#ifndef __qSlicerVolumesModule_h
#define __qSlicerVolumesModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerVolumesModuleExport.h"

class qSlicerAbstractModuleWidget;
class qSlicerVolumesModulePrivate;

/// \ingroup Slicer_QtModules_Volumes
class Q_SLICER_QTMODULES_VOLUMES_EXPORT qSlicerVolumesModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerVolumesModule(QObject *parent=0);
  virtual ~qSlicerVolumesModule();

  virtual QString helpText()const;
  virtual QString acknowledgementText()const;
  virtual QStringList contributors()const;
  virtual QIcon icon()const;
  virtual QStringList categories()const;
  virtual QStringList dependencies()const;
  qSlicerGetTitleMacro(QTMODULE_TITLE);

protected:
  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

  /// Specify editable node types
  virtual QStringList associatedNodeTypes()const;

protected:
  QScopedPointer<qSlicerVolumesModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumesModule);
  Q_DISABLE_COPY(qSlicerVolumesModule);
};

#endif
