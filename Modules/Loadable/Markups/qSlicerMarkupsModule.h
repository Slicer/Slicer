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

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerMarkupsModuleExport.h"

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

class QDockWidget;
class qSlicerMarkupsMeasurementsWidget;
class qMRMLMarkupsToolBar;
class vtkMRMLScene;
class vtkMRMLMarkupsNode;
class qSlicerMarkupsModulePrivate;
class vtkMRMLMarkupsDisplayNode;
class vtkObject;

/// \ingroup Slicer_QtModules_Markups
class Q_SLICER_QTMODULES_MARKUPS_EXPORT qSlicerMarkupsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  QVTK_OBJECT;
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
  Q_INTERFACES(qSlicerLoadableModule);
  /// Visibility of the markups toolbar
  Q_PROPERTY(bool toolBarVisible READ isToolBarVisible WRITE setToolBarVisible)
  Q_PROPERTY(bool autoShowToolBar READ autoShowToolBar WRITE setAutoShowToolBar)

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerMarkupsModule(QObject *parent=nullptr);
  ~qSlicerMarkupsModule() override;

  qSlicerGetTitleMacro(tr("Markups"));

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

  /// Indicates that markups toolbar should be showed when a new sequence is loaded.
  /// Adding a new markups node to the scene does not show the toolbar automatically
  /// but the importer must call showMarkups method.
  Q_INVOKABLE bool autoShowToolBar();
  Q_INVOKABLE bool isToolBarVisible();
  Q_INVOKABLE qMRMLMarkupsToolBar* toolBar();
  Q_INVOKABLE QDockWidget* measurementPanelDockWidget();
  Q_INVOKABLE qSlicerMarkupsMeasurementsWidget* measurementPanelWidget();

  /// Utility function for showing the markupsNode in the application user interface (toolbar)
  /// if autoShowToolBar is enabled.
  Q_INVOKABLE static bool showMarkups(vtkMRMLMarkupsNode* markupsNode);

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

public slots:
  void setToolBarVisible(bool visible);
  /// Enables automatic showing markups toolbar when a new markups node is loaded
  void setAutoShowToolBar(bool autoShow);
  void onMarkupsMeasurementsPanelToggled(bool);
  //void onNodeAddedEvent(vtkObject*, vtkObject*);

protected:
  QScopedPointer<qSlicerMarkupsModulePrivate> d_ptr;


private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsModule);
  Q_DISABLE_COPY(qSlicerMarkupsModule);

};

#endif
