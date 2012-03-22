/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLSceneViewNode.h"
#include "vtkMRMLScene.h"

#include <vtkImageData.h>


int vtkMRMLSceneViewNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLSceneViewNode > node1 = vtkSmartPointer< vtkMRMLSceneViewNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  
  EXERCISE_BASIC_MRML_METHODS(vtkMRMLSceneViewNode, node1); 

  // test with null scene
  node1->UpdateSnapshotScene(NULL);
  node1->SetAbsentStorageFileNames();
  
  // make a scene and test again
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  node1->UpdateSnapshotScene(scene);

  vtkMRMLScene *nodes = node1->GetNodes();
  std::cout << "GetNodes returned " << (nodes == NULL ? "null" : "not null") << std::endl;

  node1->SetAbsentStorageFileNames();

  TEST_SET_GET_STRING(node1, SceneViewDescription);

  node1->SetScreenShot(NULL);
  vtkImageData *nullImage = node1->GetScreenShot();
  if (nullImage != NULL)
    {
    std::cerr << "Error setting/getting a null screen shot" << std::endl;
    return EXIT_FAILURE;
    }
  vtkImageData *imageData = vtkImageData::New();
  node1->SetScreenShot(imageData);
  imageData = node1->GetScreenShot();

  imageData->Delete();
  
  TEST_SET_GET_INT_RANGE(node1, ScreenShotType, 0, 4);
  
  // scene->AddNode(node1);


  return EXIT_SUCCESS;
}
