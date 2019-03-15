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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes

// CTK includes
#include <ctkTest.h>

// Models includes
#include <qSlicerAbstractModuleRepresentation.h>
#include "qSlicerModelsModule.h"
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// Subject hierarchy widgets
#include <qMRMLSubjectHierarchyTreeView.h>

// VTK includes
#include <vtkNew.h>
#include <vtkTestingOutputWindow.h>

// --------------------------------------------------------------------------
class qSlicerModelsModuleWidgetTester: public QObject
{
  Q_OBJECT
private:

private slots:
  void testClearCurrentNode();
};

// ----------------------------------------------------------------------------
void qSlicerModelsModuleWidgetTester::testClearCurrentNode()
{
  // Create a scene with a model under hierarchy:
  // Scene
  //   + View
  //   + Hierarchy
  //       + Hierarchy -> ModelNode
  vtkNew<vtkMRMLScene> scene;

  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(
"<MRML  version=\"Slicer4.4.0\" userTags=\"\">"
" <View id=\"vtkMRMLViewNode1\"  name=\"View1\" ></View>"
" <ModelDisplay id=\"vtkMRMLModelDisplayNode4\"  name=\"ModelDisplay\"  ></ModelDisplay>"
" <Model id=\"vtkMRMLModelNode4\"  name=\"left\"  displayNodeRef=\"vtkMRMLModelDisplayNode4\"  references=\"display:vtkMRMLModelDisplayNode4;\"  ></Model>"
" <ModelDisplay id=\"vtkMRMLModelDisplayNode5\"  name=\"ModelDisplay_1\" ></ModelDisplay>"
" <ModelHierarchy id=\"vtkMRMLModelHierarchyNode1\"  name=\"Model Hierarchy\" ></ModelHierarchy>"
" <ModelHierarchy id=\"vtkMRMLModelHierarchyNode2\"  name=\"ModelHierarchy\" parentNodeRef=\"vtkMRMLModelHierarchyNode1\"  associatedNodeRef=\"vtkMRMLModelNode4\"  expanded=\"true\" ></ModelHierarchy>"
" </MRML>");
  scene->Connect();
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->GetFirstNode(nullptr, "vtkMRMLModelNode"));

  // Instantiate Models module panel
  qSlicerModelsModule module;
  TESTING_OUTPUT_ASSERT_WARNINGS_BEGIN();
  module.initialize(nullptr);
  TESTING_OUTPUT_ASSERT_WARNINGS_END(); // warning due to using 0 as application logic
  module.setMRMLScene(scene.GetPointer());

  QWidget* moduleWidget = dynamic_cast<QWidget*>(module.widgetRepresentation());
  moduleWidget->show();

  // Make the model item current
  qMRMLSubjectHierarchyTreeView* view = moduleWidget->findChild<qMRMLSubjectHierarchyTreeView*>();
  view->setCurrentNode(modelNode);

  // When the scene is cleared (EndCloseEvent), only the top-level nodes are
  // removed in the scene model of the Models tree view. The model node row is
  // not explicitly removed and therefore the QItemSelectionModel::currentRowChanged
  // signal is not emitted as it happens later when the event loop is executed.
  // If the tree view were to observe that signal it would fail to be notified
  // on time and would not stop observing the model display node (by the display
  // node widget and more specifically qMRMLDisplayNodeViewComboBox).
  // This would lead to some inconsistent state (observing a node with a null
  // scene because it has been removed by the scene) and a crash.
  scene->Clear(0);
  //qApp->exec();
}

// ----------------------------------------------------------------------------
CTK_TEST_MAIN(qSlicerModelsModuleWidgetTest)
#include "moc_qSlicerModelsModuleWidgetTest.cxx"
