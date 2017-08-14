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

#ifndef __qSlicerTransformsModule_h
#define __qSlicerTransformsModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

// Transforms includes
#include "qSlicerTransformsModuleExport.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTransformsModulePrivate;

class Q_SLICER_QTMODULES_TRANSFORMS_EXPORT qSlicerTransformsModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerTransformsModule(QObject *parent=0);
  virtual ~qSlicerTransformsModule();

  /// Icon of the transform module
  virtual QIcon icon()const;

  virtual QStringList categories()const;

  /// Dependencies of the module
  virtual QStringList dependencies()const;

  /// Display name for the module
  qSlicerGetTitleMacro("Transforms");

  /// Help text of the module
  virtual QString helpText()const;

  /// Acknowledgement of the module
  virtual QString acknowledgementText()const;

  /// Contributors of the module
  virtual QStringList contributors()const;

  /// Specify editable node types
  virtual QStringList associatedNodeTypes()const;

protected:
  /// Reimplemented to initialize the transforms IO
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

  QScopedPointer<qSlicerTransformsModulePrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qSlicerTransformsModule);
  Q_DISABLE_COPY(qSlicerTransformsModule);
};

#endif
