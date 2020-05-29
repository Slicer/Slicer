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

// Slicer includes
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
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerTransformsModule(QObject *parent=nullptr);
  ~qSlicerTransformsModule() override;

  /// Icon of the transform module
  QIcon icon()const override;

  QStringList categories()const override;

  /// Dependencies of the module
  QStringList dependencies()const override;

  /// Display name for the module
  qSlicerGetTitleMacro("Transforms");

  /// Help text of the module
  QString helpText()const override;

  /// Acknowledgement of the module
  QString acknowledgementText()const override;

  /// Contributors of the module
  QStringList contributors()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

protected:
  /// Reimplemented to initialize the transforms IO
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

  QScopedPointer<qSlicerTransformsModulePrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qSlicerTransformsModule);
  Q_DISABLE_COPY(qSlicerTransformsModule);
};

#endif
