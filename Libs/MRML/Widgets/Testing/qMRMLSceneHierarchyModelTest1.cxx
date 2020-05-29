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
#include <QApplication>
#include <QTimer>
#include <QTreeView>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include <ctkModelTester.h>

// qMRML includes
#include "qMRMLSceneFactoryWidget.h"
#include "qMRMLSceneHierarchyModel.h"
#include "qMRMLSortFilterHierarchyProxyModel.h"

// MRML includes
#include "vtkMRMLHierarchyNode.h"

// VTK includes
#include "qMRMLWidget.h"

// STD includes

int qMRMLSceneHierarchyModelTest1(int argc, char * argv [])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLSceneHierarchyModel model;
  qMRMLSceneFactoryWidget sceneFactory(nullptr);

  try
    {
    ctkModelTester tester(&model);
    tester.setTestDataEnabled(false);

    sceneFactory.generateScene();

    model.setMRMLScene(sceneFactory.mrmlScene());

    vtkMRMLNode* node1 = sceneFactory.generateNode("vtkMRMLViewNode");
    vtkMRMLHierarchyNode* hierarchyNode1 = vtkMRMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkMRMLHierarchyNode"));

    vtkMRMLHierarchyNode* hierarchyNode2 = vtkMRMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkMRMLHierarchyNode"));
    vtkMRMLNode* node2 = sceneFactory.generateNode("vtkMRMLViewNode");

    vtkMRMLHierarchyNode* hierarchyNode3 = vtkMRMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkMRMLHierarchyNode"));
    vtkMRMLNode* node3 = sceneFactory.generateNode("vtkMRMLViewNode");

    //vtkMRMLHierarchyNode* hierarchyNode4 =
    vtkMRMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkMRMLHierarchyNode"));

    hierarchyNode1->SetAssociatedNodeID(node1->GetID());
    hierarchyNode2->SetAssociatedNodeID(node3->GetID());
    hierarchyNode3->SetAssociatedNodeID(node2->GetID());

    //vtkMRMLHierarchyNode* hierarchyNode5 =
    vtkMRMLHierarchyNode::SafeDownCast(
      sceneFactory.generateNode("vtkMRMLHierarchyNode"));
    //hierarchyNode5->SetParentNodeID(hierarchyNode4->GetID());

    //vtkMRMLNode* node4 = sceneFactory.generateNode("vtkMRMLViewNode");
    //hierarchyNode5->SetAssociatedNodeID(node4->GetID());
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }

  QTreeView view(nullptr);
  view.setDragDropMode(QAbstractItemView::InternalMove);
  view.setModel(&model);
  view.show();
  view.resize(500, 300);

  QTreeView view2(nullptr);
  view2.setWindowTitle("Filtered");

  qMRMLSortFilterHierarchyProxyModel sortFilterModel;
  sortFilterModel.setNodeTypes(
    QStringList() << "vtkMRMLHierarchyNode" << "vtkMRMLViewNode" );
  sortFilterModel.setSourceModel(&model);

  view2.setDragDropMode(QAbstractItemView::InternalMove);
  view2.setModel(&sortFilterModel);
  view2.show();
  view2.resize(500, 300);

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

