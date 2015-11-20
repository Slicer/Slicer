//#include <vtkSlicerConfigure.h>
#include <vtkSlicerSceneViewsModuleLogic.h>

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkTestingOutputWindow.h>


int vtkSlicerSceneViewsModuleLogicTest1(int , char * [] )
{

  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();

  vtkSmartPointer< vtkSlicerSceneViewsModuleLogic > node1 = vtkSmartPointer< vtkSlicerSceneViewsModuleLogic >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  // should fail, no scene
  std::cout << "CreateSceneView with no mrml scene or screen shot" << std::endl;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->CreateSceneView("SceneViewTest0", "this is a scene view", 0, NULL);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  node1->SetMRMLScene(scene);
  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  std::cout << "CreateSceneView with no screenshot" << std::endl;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->CreateSceneView("SceneViewTest1", "this is a scene view", 0, NULL);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

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
  scene->SetURL(url.c_str());
  std::cout << "Reading MRML scene " << scene->GetURL() << std::endl;
  scene->Connect();
  std::cout << "After reading in MRML Scene " << url.c_str() << std::endl;
  std::cout << "\tscene has " << scene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode") << " scene view nodes" << std::endl;

  node1->SetMRMLScene(scene);
  // test trying to remove a null node
  std::cout << "Trying to remove a null node." << std::endl;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  node1->RemoveSceneViewNode(NULL);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  // add a node to remove
  node1->CreateSceneView("SceneViewTestToRemove", "this is a scene view to remove", 0, screenShot);
  vtkCollection *col = scene->GetNodesByClassByName("vtkMRMLSceneViewNode", "SceneViewTestToRemove");
  if (col && col->GetNumberOfItems() > 0)
    {
    vtkMRMLSceneViewNode *nodeToRemove = vtkMRMLSceneViewNode::SafeDownCast(col->GetItemAsObject(0));
    if (nodeToRemove)
      {
      // now remove one of the nodes
      node1->RemoveSceneViewNode(nodeToRemove);
      std::cout << "After adding and removing a scene view node, scene has " << scene->GetNumberOfNodesByClass("vtkMRMLSceneViewNode") << " scene view nodes" << std::endl;

      }
     else
      {
      std::cerr << "Error getting a scene view node to remove" << std::endl;
      return EXIT_FAILURE;
      }
    }
  else
    {
    std::cerr << "Error adding and finding a node to remove" << std::endl;
    return EXIT_FAILURE;
    }
   col->RemoveAllItems();
   col->Delete();
  return EXIT_SUCCESS;
}



