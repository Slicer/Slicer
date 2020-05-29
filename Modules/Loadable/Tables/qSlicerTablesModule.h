/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __qSlicerTablesModule_h
#define __qSlicerTablesModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerTablesModuleExport.h"

class qSlicerTablesModulePrivate;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_TABLES_EXPORT qSlicerTablesModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerTablesModule(QObject *parent=nullptr);
  ~qSlicerTablesModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  QIcon icon()const override;
  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  QStringList categories()const override;
  QStringList dependencies()const override;

  QStringList associatedNodeTypes()const override;

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerTablesModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTablesModule);
  Q_DISABLE_COPY(qSlicerTablesModule);

};

#endif
