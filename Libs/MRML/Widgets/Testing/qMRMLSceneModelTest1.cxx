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

  This file was originally developed by Luis Ibanez, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QTreeView>

// qMRML includes
#include "qMRMLSceneModel.h"
#include "qMRMLSceneFactoryWidget.h"

// MRML includes

// STD includes

int qMRMLSceneModelTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLSceneModel   sceneModel;

  if (sceneModel.rowCount() != 0)
    {
    std::cerr << "qMRMLSceneModel: wrong row count" << std::endl;
    return EXIT_FAILURE;
    }

  qMRMLSceneFactoryWidget sceneFactory;
  sceneFactory.generateScene();
  sceneModel.setMRMLScene(sceneFactory.mrmlScene());
  if (sceneModel.rowCount() != 1)
    {
    std::cerr << "qMRMLSceneModel:setScene() failed" << std::endl;
    return EXIT_FAILURE;
    }

  //sceneFactory.deleteScene();
  //sceneFactory.generateScene();
  //sceneModel.setMRMLScene(sceneFactory.mrmlScene());

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

  if (sceneModel.rowCount(sceneModel.mrmlSceneIndex()) != 11)
    {
    std::cerr << "qMRMLSceneModel wrong number of children: "
              << sceneModel.rowCount(sceneModel.mrmlSceneIndex()) << std::endl;
    return EXIT_FAILURE;
    }

  QStringList postNodes;
  postNodes << QString("temporary item");
  sceneModel.setPostItems(postNodes, sceneModel.mrmlSceneItem());
  if (sceneModel.rowCount(sceneModel.mrmlSceneIndex()) != 12)
    {
    std::cerr << "qMRMLSceneModel wrong number of children after post: "
              << sceneModel.rowCount(sceneModel.mrmlSceneIndex()) << std::endl;
    return EXIT_FAILURE;
    }

  // test if it can be replaced
  postNodes.clear();
  postNodes << "separator" << "post item 1" << "post item 2" << "post item 3";
  sceneModel.setPostItems(postNodes, sceneModel.mrmlSceneItem());

  QStringList preNodes;
  preNodes << "pre item 1" << "pre item 2"  << "separator";
  sceneModel.setPreItems(preNodes, sceneModel.mrmlSceneItem());

  QStringList postScene;
  postScene << "post scene item";
  sceneModel.setPostItems(postScene, 0);

  if (sceneModel.rowCount(sceneModel.mrmlSceneIndex()) != 18)
    {
    std::cerr << "qMRMLSceneModel wrong number of children after pre/post: "
              << sceneModel.rowCount(sceneModel.mrmlSceneIndex()) << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();

  if (sceneModel.rowCount(sceneModel.mrmlSceneIndex()) != 21)
    {
    std::cerr << "qMRMLSceneModel wrong number of children after adding nodes: "
              << sceneModel.rowCount(sceneModel.mrmlSceneIndex()) << std::endl;
    return EXIT_FAILURE;
    }

  if (sceneModel.columnCount() != 1 ||
      sceneModel.columnCount(sceneModel.mrmlSceneIndex()) != 1)
    {
    std::cerr << "Wrong number of columns" << sceneModel.columnCount()
              << " " << sceneModel.columnCount(sceneModel.mrmlSceneIndex()) << std::endl;
    return EXIT_FAILURE;
    }

  QTreeView* view = new QTreeView(0);
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  view->setSelectionBehavior(QAbstractItemView::SelectRows);
  view->setDragDropMode(QAbstractItemView::InternalMove);
  view->setModel(&sceneModel);
  view->show();

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}
