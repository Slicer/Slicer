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
#include <QWidget>

// Slicer includes
#include "qMRMLLayoutManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

int qMRMLLayoutManagerTest1(int argc, char * argv[] )
{
  QApplication app(argc, argv);
  qMRMLLayoutManager * layoutManager = new qMRMLLayoutManager();

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;

  {
    vtkNew<vtkMRMLScene> scene;
    applicationLogic->SetMRMLScene(scene.GetPointer());
    layoutManager->setMRMLScene(scene.GetPointer());
    if (layoutManager->mrmlScene() != scene.GetPointer())
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager::setMRMLScene()" << std::endl;
      return EXIT_FAILURE;
      }
    layoutManager->setMRMLScene(0);
    applicationLogic->SetMRMLScene(0);
    if (layoutManager->mrmlScene() != 0)
      {
      std::cerr << "Line " << __LINE__ << " - Problem with qMRMLLayoutManager::setMRMLScene()" << std::endl;
      return EXIT_FAILURE;
      }
  }

  vtkNew<vtkMRMLScene> scene;
  applicationLogic->SetMRMLScene(scene.GetPointer());
  layoutManager->setMRMLScene(scene.GetPointer());

  QWidget * viewport = new QWidget;
  viewport->setWindowTitle("Old widget");
  layoutManager->setViewport(viewport);
  viewport->show();
  layoutManager->setViewport(0);
  layoutManager->setViewport(viewport);

  QWidget * viewport2 = new QWidget;
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
  delete layoutManager;
  delete viewport;
  delete viewport2;
  return res;
}

