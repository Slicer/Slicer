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

#ifndef __qSlicerSequencesModule_h
#define __qSlicerSequencesModule_h

// CTK includes
#include <ctkVTKObject.h>

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

#include "qSlicerSequencesModuleExport.h"

class qMRMLSequenceBrowserToolBar;
class vtkMRMLScene;
class vtkMRMLSequenceBrowserNode;
class vtkObject;


class qSlicerSequencesModulePrivate;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_SEQUENCES_EXPORT
qSlicerSequencesModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
  QVTK_OBJECT;
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

  /// Visibility of the sequence browser toolbar
  Q_PROPERTY(bool toolBarVisible READ isToolBarVisible WRITE setToolBarVisible)
  Q_PROPERTY(bool autoShowToolBar READ autoShowToolBar WRITE setAutoShowToolBar)

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerSequencesModule(QObject *parent=0);
  ~qSlicerSequencesModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE);

  QString helpText()const override;
  QString acknowledgementText()const override;
  QStringList contributors()const override;

  QIcon icon()const override;

  QStringList categories()const override;
  QStringList dependencies() const override;

  /// Specify editable node types
  QStringList associatedNodeTypes()const override;

  /// Indicates that sequence browser toolbar should be showed when a new sequence is loaded.
  /// Adding a new sequence browser node to the scene does not show the toolbar automatically
  /// but the importer must call showSequenceBrowser method.
  Q_INVOKABLE bool autoShowToolBar();
  Q_INVOKABLE bool isToolBarVisible();
  Q_INVOKABLE qMRMLSequenceBrowserToolBar* toolBar();

  /// Utility function for showing the browserNode in the application user interface (toolbar)
  /// if autoShowToolBar is enabled.
  Q_INVOKABLE static bool showSequenceBrowser(vtkMRMLSequenceBrowserNode* browserNode);

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

public slots:
  void setMRMLScene(vtkMRMLScene*) override;
  void setToolBarVisible(bool visible);
  /// Enables automatic showing sequence browser toolbar when a new sequence is loaded
  void setAutoShowToolBar(bool autoShow);
  void onNodeAddedEvent(vtkObject*, vtkObject*);
  void onNodeRemovedEvent(vtkObject*, vtkObject*);
  void updateAllVirtualOutputNodes();

  void setToolBarActiveBrowserNode(vtkMRMLSequenceBrowserNode* browserNode);

protected:
  QScopedPointer<qSlicerSequencesModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSequencesModule);
  Q_DISABLE_COPY(qSlicerSequencesModule);

};

#endif
