/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSceneViewNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>


int vtkMRMLSceneViewNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLSceneViewNode> node1;

  EXERCISE_BASIC_OBJECT_METHODS(node1.GetPointer());

  EXERCISE_BASIC_MRML_METHODS(vtkMRMLSceneViewNode, node1.GetPointer());

  // test with null scene
  node1->StoreScene();
  node1->SetAbsentStorageFileNames();
  vtkCollection *col = node1->GetNodesByClass(NULL);
  if (col != NULL)
    {
    std::cout << "Failed to get empty collection" << std::endl;
    return EXIT_FAILURE;
    }

  // make a scene and test again
  vtkNew<vtkMRMLScene> scene;
  node1->SetScene(scene.GetPointer());
  node1->StoreScene();

  vtkMRMLScene *storedScene = node1->GetStoredScene();
  std::cout << "GetStoredScene returned " << (storedScene == NULL ? "null" : "not null") << std::endl;

  node1->SetAbsentStorageFileNames();

  TEST_SET_GET_STRING(node1.GetPointer(), SceneViewDescription);

  node1->SetScreenShot(NULL);
  vtkImageData *nullImage = node1->GetScreenShot();
  if (nullImage != NULL)
    {
    std::cerr << "Error setting/getting a null screen shot" << std::endl;
    return EXIT_FAILURE;
    }
  vtkImageData *imageData = vtkImageData::New();
  node1->SetScreenShot(imageData);
  imageData->Delete();
  imageData = node1->GetScreenShot();

  TEST_SET_GET_INT_RANGE(node1.GetPointer(), ScreenShotType, 0, 4);

  col = node1->GetNodesByClass("vtkMRMLNode");
  if (!col)
    {
    std::cerr << "Failed to get mrml nodes from scene view snapshot scene" << std::endl;
    return EXIT_FAILURE;
    }
  col->RemoveAllItems();
  col->Delete();

  return EXIT_SUCCESS;
}
