/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QTreeView>

// qMRML includes
#include "qMRMLSceneModel2.h"
#include "qMRMLSceneFactoryWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLSceneModelTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLSceneModel2   sceneModel;

  if (sceneModel.rowCount() != 0)
    {
    std::cerr << "qMRMLSceneModel2: wrong row count" << std::endl;
    return EXIT_FAILURE;
    }

  qMRMLSceneFactoryWidget sceneFactory;
  sceneFactory.generateScene();
  sceneModel.setMRMLScene(sceneFactory.mrmlScene());
  if (sceneModel.rowCount() != 1)
    {
    std::cerr << "qMRMLSceneModel2:setScene() failed" << std::endl;
    return EXIT_FAILURE;
    }

  QStringList preNodes;
  preNodes << "pre item 1" << "pre item 2"  << "separator";
  sceneModel.setPreItems(preNodes, sceneModel.mrmlSceneItem());

  QStringList postScene;
  postScene << "post scene item";
  sceneModel.setPostItems(postScene, 0);

  if (sceneModel.rowCount(sceneModel.mrmlSceneIndex()) != 18)
    {
    std::cerr << "qMRMLSceneModel2 wrong number of children after pre/post: "
              << sceneModel.rowCount(sceneModel.mrmlSceneIndex()) << std::endl;
    return EXIT_FAILURE;
    }

  sceneFactory.generateNode();
  sceneFactory.generateNode();
  sceneFactory.generateNode();

  if (sceneModel.rowCount(sceneModel.mrmlSceneIndex()) != 21)
    {
    std::cerr << "qMRMLSceneModel2 wrong number of children after adding nodes: "
              << sceneModel.rowCount(sceneModel.mrmlSceneIndex()) << std::endl;
    return EXIT_FAILURE;
    }

  if (sceneModel.columnCount() != 2 ||
      sceneModel.columnCount(sceneModel.mrmlSceneIndex()) != 2)
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

  return app.exec();
}
