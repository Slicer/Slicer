/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __qSlicerTextsModule_h
#define __qSlicerTextsModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

// Texts includes
#include "qSlicerTextsModuleExport.h"

class vtkMatrix4x4;
class vtkMRMLNode;
class qSlicerTextsModulePrivate;

class Q_SLICER_QTMODULES_TEXTS_EXPORT qSlicerTextsModule : public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);
public:

  typedef qSlicerLoadableModule Superclass;
  qSlicerTextsModule(QObject *parent=nullptr);
  ~qSlicerTextsModule() override;

  /// Icon of the Texts module
  QIcon icon()const override;

  /// Categories where the module should appear
  QStringList categories()const override;

  /// Dependencies of the module
  QStringList dependencies()const override;

  /// Display name for the module
  qSlicerGetTitleMacro("Texts");

  /// Help text of the module
  QString helpText()const override;

  /// Acknowledgement of the module
  QString acknowledgementText()const override;

  /// Contributors of the module
  QStringList contributors()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

protected:
  /// Reimplemented to initialize the Texts IO
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

  QScopedPointer<qSlicerTextsModulePrivate> d_ptr;
private:
  Q_DECLARE_PRIVATE(qSlicerTextsModule);
  Q_DISABLE_COPY(qSlicerTextsModule);
};

#endif
