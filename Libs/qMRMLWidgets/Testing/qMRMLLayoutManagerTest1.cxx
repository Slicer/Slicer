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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QTimer>
#include <QWidget>

// SlicerQt includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLScene.h>

// STD includes
#include <cstdlib>

int qMRMLLayoutManagerTest1(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  qMRMLLayoutManager* layoutManager = new qMRMLLayoutManager(0);

  vtkMRMLScene* scene = vtkMRMLScene::New();
  layoutManager->setMRMLScene(scene);
  if (layoutManager->mrmlScene() != scene)
    {
    std::cerr << "scene incorrectly set." << std::endl;
    return EXIT_FAILURE;
    }
  layoutManager->setMRMLScene(0);
  scene->Delete();
  scene = vtkMRMLScene::New();
  layoutManager->setMRMLScene(scene);
  QWidget* viewport = new QWidget(0);
  viewport->setWindowTitle("Old widget");
  layoutManager->setViewport(viewport);
  viewport->show();

  layoutManager->setViewport(0);
  layoutManager->setViewport(viewport);

  QWidget* viewport2 = new QWidget(0);
  viewport2->setWindowTitle("New widget");
  layoutManager->setViewport(viewport2);
  viewport2->show();

  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }
  int res = app.exec();

  scene->Delete();
  delete layoutManager;
  delete viewport;
  delete viewport2;
  return res;
}

