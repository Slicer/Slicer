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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerUnitsModule_h
#define __qSlicerUnitsModule_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerUnitsModuleExport.h"

class qSlicerUnitsModulePrivate;

class Q_SLICER_QTMODULES_UNITS_EXPORT qSlicerUnitsModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerUnitsModule(QObject *parent=nullptr);
  ~qSlicerUnitsModule() override;

  qSlicerGetTitleMacro("Units");

  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  QIcon icon()const override;

  QStringList categories()const override;
  QStringList dependencies() const override;

  /// Hide unit module by default
  bool isHidden() const override;

protected:
  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerUnitsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerUnitsModule);
  Q_DISABLE_COPY(qSlicerUnitsModule);

};

#endif
