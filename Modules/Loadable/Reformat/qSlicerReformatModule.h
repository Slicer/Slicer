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

  This file was originally developed by Michael Jeulin-Lagarrigue, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerReformatModule_h
#define __qSlicerReformatModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerReformatModuleExport.h"

class qSlicerReformatModulePrivate;

/// \ingroup Slicer_QtModules_Reformat
class Q_SLICER_QTMODULES_REFORMAT_EXPORT
qSlicerReformatModule : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerReformatModule(QObject *parent=nullptr);
  ~qSlicerReformatModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  QString helpText()const override;

  /// Return acknowledgements
  QString acknowledgementText()const override;

  /// Return a custom icon for the module
  QIcon icon()const override;

  /// Return the category for the module
  QStringList categories()const override;

  /// Return the contributor for the module
  QStringList contributors()const override;

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
  QScopedPointer<qSlicerReformatModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerReformatModule);
  Q_DISABLE_COPY(qSlicerReformatModule);

};

#endif
