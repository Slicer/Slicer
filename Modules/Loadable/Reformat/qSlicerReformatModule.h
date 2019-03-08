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

// SlicerQt includes
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
  explicit qSlicerReformatModule(QObject *parent=0);
  virtual ~qSlicerReformatModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  virtual QString helpText()const;

  /// Return acknowledgements
  virtual QString acknowledgementText()const;

  /// Return a custom icon for the module
  virtual QIcon icon()const;

  /// Return the category for the module
  virtual QStringList categories()const;

  /// Return the contributor for the module
  virtual QStringList contributors()const;

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
  QScopedPointer<qSlicerReformatModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerReformatModule);
  Q_DISABLE_COPY(qSlicerReformatModule);

};

#endif
