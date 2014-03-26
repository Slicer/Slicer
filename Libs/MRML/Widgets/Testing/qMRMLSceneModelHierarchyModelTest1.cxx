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

// QT includes
#include <QApplication>
#include <QTimer>
#include <QTreeView>

// CTK includes
#include <ctkModelTester.h>

// qMRML includes
#include "qMRMLSceneFactoryWidget.h"
#include "qMRMLSceneModelHierarchyModel.h"


// STD includes

int qMRMLSceneModelHierarchyModelTest1(int argc, char * argv [])
{
  QApplication app(argc, argv);

  qMRMLSceneModelHierarchyModel model;
  qMRMLSceneFactoryWidget sceneFactory(0);

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

    vtkMRMLNode* node1 = sceneFactory.generateNode("vtkMRMLModelHierarchyNode");
    vtkMRMLNode* node2 = sceneFactory.generateNode("vtkMRMLModelHierarchyNode");
    std::cout << "start reparenting node" << std::endl;
    model.reparent(node1, node2);
    std::cout << "end reparenting node" << std::endl;
    vtkMRMLNode* node3 = sceneFactory.generateNode("vtkMRMLModelNode");
    vtkMRMLNode* node4 = sceneFactory.generateNode("vtkMRMLModelHierarchyNode");
    std::cout << "start reparenting node 2" << std::endl;
    model.reparent(node3, node4);
    std::cout << "end reparenting node 2" << std::endl;
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
  QTreeView* view = new QTreeView(0);
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

