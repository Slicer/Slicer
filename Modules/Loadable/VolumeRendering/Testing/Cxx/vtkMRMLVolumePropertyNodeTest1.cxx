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


namespace
{
int readWrite();
int piecewiseFunctionFromString();
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNodeTest1(int argc, char *[] )
{
  if (argc < 2)
    {
    std::cout << "Usage: vtkMRMLVolumePropertyNodeTest1 scene_file_path.mrml"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkNew<vtkMRMLVolumePropertyNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  CHECK_EXIT_SUCCESS(readWrite());
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
int readWrite()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLVolumePropertyNode> propertyNode;
  scene->RegisterNodeClass(propertyNode.GetPointer());

  vtkNew<vtkPiecewiseFunction> scalarOpacity;
  scalarOpacity->AddPoint(-10000., 0.);
  scalarOpacity->AddPoint(-10001., 1.);
  scalarOpacity->AddPoint(0., 0.00001);
  scalarOpacity->AddPoint(0.1, 1.0000);
  scalarOpacity->AddPoint(7.0, 0.122);
  // precision is important as it is used for "threshold" (points are very
  // close to each other).
  scalarOpacity->AddPoint(7.0000000000001, 1.0);
  scalarOpacity->AddPoint(7.0000000000001 + std::numeric_limits<double>::epsilon(), 0.01);
  scalarOpacity->AddPoint(10., 0.000000001);

  propertyNode->SetScalarOpacity(scalarOpacity.GetPointer());
  scene->AddNode(propertyNode.GetPointer());

  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string sceneXML = scene->GetSceneXMLString();

  vtkNew<vtkMRMLScene> scene2;
  scene2->RegisterNodeClass(propertyNode.GetPointer());
  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(sceneXML);
  scene2->Import();

  vtkMRMLVolumePropertyNode* propertyNode2 =
    vtkMRMLVolumePropertyNode::SafeDownCast(
    scene2->GetFirstNodeByClass("vtkMRMLVolumePropertyNode"));
  CHECK_NOT_NULL(propertyNode2);

  vtkPiecewiseFunction* scalarOpacity2 = propertyNode2->GetScalarOpacity();
  CHECK_NOT_NULL(scalarOpacity2);

  for (int i = 0; i < scalarOpacity->GetSize(); ++i)
    {
    double value[4];
    double value2[4];
    int res = scalarOpacity->GetNodeValue(i, value);
    int res2 = scalarOpacity2->GetNodeValue(i, value2);
    if (res == -1 || res2 == -1
        || value[0] != value2[0]
        || value[1] != value2[1]
        || value[2] != value2[2]
        || value[3] != value2[3]
        )
        {
        std::cout << __FUNCTION__ << ":" << __LINE__ << "failed:" << std::endl
                  << "  Scalar opacity values are different:" << std::endl
                  << "     " << res << " " << value[0] << " " << value[1]
                  << " " << value[2] << " " << value[3] << std::endl
                  << "  instead of" << std::endl
                  << "     " << res << " " << value[0] << " " << value[1]
                  << " " << value[2] << " " << value[3] << std::endl;
        return EXIT_FAILURE;
        }
    }
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
int piecewiseFunctionFromString()
{
  std::string s("10 0 0 4.94065645841247e-324 0 69.5504608154297"
                " 0 154.266067504883 0.699999988079071 228 1");
  double expectedData[10] = {0, 0, 4.94065645841247e-324, 0, 69.5504608154297,
                             0, 154.266067504883, 0.699999988079071, 228, 1};
  vtkSmartPointer<vtkPiecewiseFunction> function =
    vtkSmartPointer<vtkPiecewiseFunction>::New();
  vtkMRMLVolumePropertyNode::GetPiecewiseFunctionFromString(s, function);
  CHECK_INT(function->GetSize(), 5);

  for (int i=0; i < 5; ++i)
    {
    double node[4];
    function->GetNodeValue(i, node);
    if (node[0] != expectedData[i*2] ||
        node[1] != expectedData[i*2+1])
      {
      std::cout << "Failed to parse value at index " << i << ", "
                << "found [" << node[0] << "," << node[1] << "] "
                << "instead of [" << expectedData[i*2] << ","
                << expectedData[i*2+1] << "]" << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}

}
