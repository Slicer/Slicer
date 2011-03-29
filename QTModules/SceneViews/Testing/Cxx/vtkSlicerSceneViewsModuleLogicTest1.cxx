//#include <vtkSlicerConfigure.h>
#include <vtkSlicerSceneViewsModuleLogic.h>

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include "TestingMacros.h"
#include <time.h>
#include <vtkImageData.h>

int vtkSlicerSceneViewsModuleLogicTest1(int , char * [] )
{

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkSmartPointer< vtkSlicerSceneViewsModuleLogic > node1 = vtkSmartPointer< vtkSlicerSceneViewsModuleLogic >::New();  
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  // should fail, no scene
  std::cout << "CreateSceneView with no mrml scene or screen shot" << std::endl;
  node1->CreateSceneView("SceneViewTest0", "this is a scene view", 0, NULL);
  
  node1->SetMRMLScene(scene);
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  std::cout << "CreateSceneView with no screenshot" << std::endl;
  node1->CreateSceneView("SceneViewTest1", "this is a scene view", 0, NULL);

  // should pass w/screen shot
  vtkSmartPointer< vtkImageData > screenShot = vtkSmartPointer< vtkImageData >::New();
  std::cout << "CreateSceneView with no name or description, with screen shot" << std::endl;
  node1->CreateSceneView("", "", 1, screenShot);

  // give a name
  std::cout << "CreateSceneView with name, no description, with screen shot" << std::endl;
  node1->CreateSceneView("SceneViewTest2", "", 2, screenShot);

  std::cout << "MRML Scene has " << scene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode") << " scene view nodes" << std::endl;

  std::string url = std::string("SceneViewsModuleTest.mrml");
  scene->SetURL(url.c_str());
  std::cout << "Writing MRML scene " << url.c_str() << std::endl;
  scene->Commit();

  // now reload it
  vtkSmartPointer<vtkMRMLScene> sceneRead = vtkSmartPointer<vtkMRMLScene>::New();
  sceneRead->SetURL(url.c_str());
  if (1)
    {
    std::cout << "Reading MRML scene " << sceneRead->GetURL() << std::endl;
    sceneRead->Connect();
    std::cout << "After reading in MRML Scene " << url.c_str() << std::endl;
    std::cout << "\tscene has " << scene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode") << " scene view nodes" << std::endl;
    }
  
  return EXIT_SUCCESS;  
}



