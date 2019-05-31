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
#include <QApplication>

#include <qMRMLTreeView.h>
#include <qMRMLSceneTransformModel.h>

// Slicer includes
#include "vtkSlicerConfigure.h"

// VTK includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>
#include "qMRMLWidget.h"

#include <vtkTimerLog.h>

// STD includes

int qMRMLTreeViewTest1( int argc, char * argv [] )
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();
  if( argc < 2 )
    {
    std::cerr << "Error: missing arguments" << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << "  inputURL_scene.mrml " << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << std::endl<< "***************************************" << std::endl;
  vtkTimerLog* timer = vtkTimerLog::New();
  vtkMRMLScene* scene = vtkMRMLScene::New();
  vtkMRMLApplicationLogic* applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << std::endl << "Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << std::endl << "Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  qMRMLSceneModel   sceneModel;
  sceneModel.setMRMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLSceneModel Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLSceneModel Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  qMRMLSceneTransformModel   transformModel;
  transformModel.setMRMLScene(scene);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLSceneTransformModel Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLSceneTransformModel Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  qMRMLSceneTransformModel   transformModel2;
  transformModel2.setMRMLScene(scene);
  qMRMLSortFilterProxyModel  sortModel;
  sortModel.setSourceModel(&transformModel2);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLSceneTransformModel(+qMRMLSortFilterProxyModel) Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLSceneTransformModel(+qMRMLSortFilterProxyModel) Deleted: " << timer->GetElapsedTime() << std::endl;


  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  qMRMLTreeView   mrmlItem;
  mrmlItem.setMRMLScene(scene);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLTreeView Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLTreeView Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  applicationLogic = vtkMRMLApplicationLogic::New();
  applicationLogic->SetMRMLScene(scene);
  qMRMLTreeView   treeWidget;
  treeWidget.show();
  treeWidget.setMRMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLTreeView visible Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  applicationLogic->Delete();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLTreeView visible Deleted: " << timer->GetElapsedTime() << std::endl;

  timer->Delete();
  return EXIT_SUCCESS;
}
