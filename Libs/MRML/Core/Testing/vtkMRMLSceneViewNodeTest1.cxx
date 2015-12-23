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
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  // test with null scene
  node1->StoreScene();
  node1->SetAbsentStorageFileNames();
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  vtkCollection *col = node1->GetNodesByClass(NULL);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  CHECK_NULL(col);

  // make a scene and test again
  vtkNew<vtkMRMLScene> scene;
  node1->SetScene(scene.GetPointer());
  node1->StoreScene();

  vtkMRMLScene *storedScene = node1->GetStoredScene();
  std::cout << "GetStoredScene returned " << (storedScene == NULL ? "null" : "not null") << std::endl;

  node1->SetAbsentStorageFileNames();

  TEST_SET_GET_STRING( node1.GetPointer(), SceneViewDescription);

  node1->SetScreenShot(NULL);
  vtkImageData *nullImage = node1->GetScreenShot();
  CHECK_NULL(nullImage);

  vtkImageData *imageData = vtkImageData::New();
  node1->SetScreenShot(imageData);
  imageData->Delete();
  imageData = node1->GetScreenShot();

  TEST_SET_GET_INT_RANGE( node1.GetPointer(), ScreenShotType, 0, 4);

  col = node1->GetNodesByClass("vtkMRMLNode");
  CHECK_NOT_NULL(col);

  col->RemoveAllItems();
  col->Delete();

  return EXIT_SUCCESS;
}
