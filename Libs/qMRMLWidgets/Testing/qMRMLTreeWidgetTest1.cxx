#include <QApplication>

#include <qMRMLTreeWidget.h>
#include <qMRMLSceneModel.h>
#include <qMRMLSceneTransformModel.h>
#include <qMRMLSortFilterProxyModel.h>

#include <vtkMRMLScene.h>

#include <vtkTimerLog.h>

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLTreeWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);
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
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << std::endl << "Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  scene->Delete();
  timer->StopTimer();
  std::cout << std::endl << "Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  qMRMLSceneModel   sceneModel;
  sceneModel.setMRMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLSceneModel Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLSceneModel Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  qMRMLSceneTransformModel   transformModel;
  transformModel.setMRMLScene(scene);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLSceneTransformModel Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLSceneTransformModel Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
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
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLSceneTransformModel(+qMRMLSortFilterProxyModel) Deleted: " << timer->GetElapsedTime() << std::endl;


  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  qMRMLTreeWidget   mrmlItem;
  mrmlItem.setMRMLScene(scene);
  scene->SetURL(argv[1]);
   timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLTreeWidget Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLTreeWidget Deleted: " << timer->GetElapsedTime() << std::endl;

  std::cout << std::endl<< "***************************************" << std::endl;
  scene = vtkMRMLScene::New();
  qMRMLTreeWidget   treeWidget;
  treeWidget.show();
  treeWidget.setMRMLScene(scene);
  scene->SetURL(argv[1]);
  timer->StartTimer();
  scene->Import();
  timer->StopTimer();
  std::cout << "qMRMLTreeWidget visible Loaded: " << timer->GetElapsedTime() << std::endl;
  timer->StartTimer();
  scene->Delete();
  timer->StopTimer();
  std::cout << "qMRMLTreeWidget visible Deleted: " << timer->GetElapsedTime() << std::endl;

  timer->Delete();
  return EXIT_SUCCESS;
}
