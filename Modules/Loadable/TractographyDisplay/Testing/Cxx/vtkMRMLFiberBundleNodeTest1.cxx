/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLCommandLineModuleNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkSmartPointer.h"

int vtkMRMLFiberBundleNodeTest1(int argc, char * argv[] )
{
  if (argc < 2)
    {
    std::cout << "Usage: vtkMRMLFiberBundleNodeTest1 scene_file_path.mrml"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLFiberBundleNode> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  const char* sceneFilePath = argv[1];
  vtkNew<vtkMRMLScene> scene;
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLCommandLineModuleNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLFiberBundleGlyphDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLFiberBundleLineDisplayNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLFiberBundleStorageNode>::New());
  scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLFiberBundleTubeDisplayNode>::New());

  CHECK_EXIT_SUCCESS(vtkMRMLCoreTestingUtilities::ExerciseSceneLoadingMethods(sceneFilePath, scene.GetPointer()));

  return EXIT_SUCCESS;
}
