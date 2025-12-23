/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// Volume Rendering includes
#include "vtkMRMLVolumePropertyNode.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLScene.h>

#include "vtkSlicerMarkupsLogic.h"
#include "vtkSlicerVolumeRenderingLogic.h"

// VTK includes
#include <vtkPiecewiseFunction.h>

// STD includes
#include <limits>
#include <iostream>

namespace
{
int readOldScene(const char*);
int piecewiseFunctionFromString();
} // namespace

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNodeTest1(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: vtkMRMLVolumePropertyNodeTest1 scene_file_path.mrml" << std::endl;
    return EXIT_FAILURE;
  }

  const char* sceneFilePath = argv[1];

  vtkNew<vtkMRMLVolumePropertyNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(readOldScene(sceneFilePath));
  CHECK_EXIT_SUCCESS(piecewiseFunctionFromString());

  vtkNew<vtkMRMLApplicationLogic> applicationLogic;
  applicationLogic->SetMRMLScene(scene.GetPointer()); // register custom nodes
  vtkNew<vtkSlicerVolumeRenderingLogic> vrLogic;
  vrLogic->SetMRMLScene(scene.GetPointer()); // register custom nodes
  vtkNew<vtkSlicerMarkupsLogic> markupsLogic;
  markupsLogic->SetMRMLScene(scene.GetPointer()); // register custom nodes
  vtkNew<vtkMRMLLayoutLogic> layLogic;
  layLogic->SetMRMLScene(scene.GetPointer()); // register custom nodes

  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
int readOldScene(const char* mrmlScenePath)
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLVolumePropertyNode> propertyNode;
  scene->RegisterNodeClass(propertyNode.GetPointer());
  scene->SetURL(mrmlScenePath);
  scene->Connect();

  vtkMRMLVolumePropertyNode* vpNode = vtkMRMLVolumePropertyNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLVolumePropertyNode"));

  vtkNew<vtkPiecewiseFunction> expectedScalarOpacity;
  expectedScalarOpacity->AddPoint(-10000., 0.);
  expectedScalarOpacity->AddPoint(-10001., 1.);
  expectedScalarOpacity->AddPoint(0., 0.00001);
  expectedScalarOpacity->AddPoint(0.1, 1.0000);
  expectedScalarOpacity->AddPoint(7.0, 0.122);
  // precision is important as it is used for "threshold" (points are very
  // close to each other).
  expectedScalarOpacity->AddPoint(7.000000000001, 1.0);
  expectedScalarOpacity->AddPoint(10., 0.000000001);

  vtkPiecewiseFunction* actualScalarOpacity = vpNode->GetScalarOpacity();
  CHECK_NOT_NULL(actualScalarOpacity);

  CHECK_INT(actualScalarOpacity->GetSize(), expectedScalarOpacity->GetSize());
  for (int i = 0; i < actualScalarOpacity->GetSize(); ++i)
  {
    double actualValue[4];
    double expectedValue[4];
    int actual = actualScalarOpacity->GetNodeValue(i, actualValue);
    CHECK_INT(actual, 1);
    int expected = expectedScalarOpacity->GetNodeValue(i, expectedValue);
    CHECK_INT(expected, 1);
    CHECK_DOUBLE_TOLERANCE(actualValue[0], expectedValue[0], std::numeric_limits<double>::epsilon())
    CHECK_DOUBLE_TOLERANCE(actualValue[1], expectedValue[1], std::numeric_limits<double>::epsilon())
    CHECK_DOUBLE_TOLERANCE(actualValue[2], expectedValue[2], std::numeric_limits<double>::epsilon())
    CHECK_DOUBLE_TOLERANCE(actualValue[3], expectedValue[3], std::numeric_limits<double>::epsilon())
  }
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int piecewiseFunctionFromString()
{
  std::string s("10 0 0 4.94065645841247e-324 0 69.5504608154297"
                " 0 154.266067504883 0.699999988079071 228 1");
  double expectedData[10] = { 0, 0, 4.94065645841247e-324, 0, 69.5504608154297, 0, 154.266067504883, 0.699999988079071, 228, 1 };
  vtkSmartPointer<vtkPiecewiseFunction> function = vtkSmartPointer<vtkPiecewiseFunction>::New();
  vtkMRMLVolumePropertyNode::GetPiecewiseFunctionFromString(s, function);
  CHECK_INT(function->GetSize(), 5);

  for (int i = 0; i < 5; ++i)
  {
    double node[4];
    function->GetNodeValue(i, node);
    if (node[0] != expectedData[i * 2] || node[1] != expectedData[i * 2 + 1])
    {
      std::cout << "Failed to parse value at index " << i << ", "
                << "found [" << node[0] << "," << node[1] << "] "
                << "instead of [" << expectedData[i * 2] << "," << expectedData[i * 2 + 1] << "]" << std::endl;
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

} // namespace
