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
#include "qMRMLSceneDisplayableModel.h"

// VTK includes
#include "qMRMLWidget.h"

// STD includes

// MRML includes
#include <vtkMRMLDisplayableHierarchyNode.h>

int qMRMLSceneDisplayableModelTest1(int argc, char * argv [])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLSceneDisplayableModel model;
  qMRMLSceneFactoryWidget sceneFactory(nullptr);

  try
    {
    ctkModelTester tester(&model);
    tester.setTestDataEnabled(false);

    sceneFactory.generateScene();

    model.setMRMLScene(sceneFactory.mrmlScene());
    model.setMRMLScene(sceneFactory.mrmlScene());

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.generateNode();

    sceneFactory.deleteNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.deleteNode();

    sceneFactory.generateNode();
    sceneFactory.generateNode();

    sceneFactory.deleteNode();
    sceneFactory.deleteNode();

    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.deleteNode();
      }
    for( int i = 0; i < 100; ++i)
      {
      sceneFactory.generateNode();
      }
    for( int i = 0; i < 99; ++i)
      {
      sceneFactory.deleteNode();
      }
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();
    sceneFactory.generateNode();

    vtkMRMLNode* node1 = sceneFactory.generateNode("vtkMRMLModelNode");
    vtkMRMLNode* node2 = sceneFactory.generateNode("vtkMRMLScalarVolumeNode");
    vtkMRMLNode* hnode1 = sceneFactory.generateNode("vtkMRMLModelHierarchyNode");
    vtkMRMLDisplayableHierarchyNode* h1 = vtkMRMLDisplayableHierarchyNode::SafeDownCast(hnode1);
    vtkMRMLNode* hnode2 = sceneFactory.generateNode("vtkMRMLDisplayableHierarchyNode");
    vtkMRMLDisplayableHierarchyNode* h2 = vtkMRMLDisplayableHierarchyNode::SafeDownCast(hnode2);
    if (node1 && node1->GetID() && h1)
      {
      std::cout << "Adding node1 to a hierarchy node" << std::endl;
      h1->SetDisplayableNodeID(node1->GetID());
      if (node2 && node2->GetID() && h2)
        {
        std::cout << "Adding node2 to a hierarchy node" << std::endl;
        h2->SetDisplayableNodeID(node2->GetID());
        }
      if (h2 && h2->GetID())
        {
        h1->SetParentNodeID(h2->GetID());
        }
      }
    std::cout << "start reparenting node" << std::endl;
    model.qMRMLSceneDisplayableModel::reparent(node1, node2);
    std::cout << "end reparenting node" << std::endl;
    }
  catch (const char* error)
    {
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
    }
  /*
  QStandardItemModel m;
  m.setColumnCount(2);
  QStandardItem* item = new QStandardItem("titi");
  m.insertRow(0, item);
  QList<QStandardItem*> items;
  items << new QStandardItem("toto");
  items << new QStandardItem("tata");
  items[0]->setBackground(QLinearGradient());
  item->insertRow(0,items);
  */
  QTreeView* view = new QTreeView(nullptr);
  //view->setSelectionBehavior(QAbstractItemView::SelectRows);
  view->setDragDropMode(QAbstractItemView::InternalMove);
  view->setModel(&model);
  //view->setModel(&m);
  view->show();
  view->resize(500, 800);

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

