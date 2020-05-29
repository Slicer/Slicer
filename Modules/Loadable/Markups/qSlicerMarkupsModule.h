/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerMarkupsModule_h
#define __qSlicerMarkupsModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerMarkupsModuleExport.h"

class qSlicerMarkupsModulePrivate;
class vtkMRMLMarkupsDisplayNode;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_QTMODULES_MARKUPS_EXPORT qSlicerMarkupsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerMarkupsModule(QObject *parent=nullptr);
  ~qSlicerMarkupsModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  /// Help to use the module
  QString helpText()const override;

  /// Return acknowledgements
  QString acknowledgementText()const override;

  /// Return the authors of the module
  QStringList  contributors()const override;

  /// Return a custom icon for the module
  QIcon icon()const override;

  /// Return the categories for the module
  QStringList categories()const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

  void setMRMLScene(vtkMRMLScene* scene) override;

  static void readDefaultMarkupsDisplaySettings(vtkMRMLMarkupsDisplayNode* markupsDisplayNode);
  static void writeDefaultMarkupsDisplaySettings(vtkMRMLMarkupsDisplayNode* markupsDisplayNode);

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

  QScopedPointer<qSlicerMarkupsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsModule);
  Q_DISABLE_COPY(qSlicerMarkupsModule);

};

#endif
