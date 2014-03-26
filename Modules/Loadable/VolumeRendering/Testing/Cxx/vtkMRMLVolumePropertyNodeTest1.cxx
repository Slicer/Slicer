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
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkPiecewiseFunction.h>

// STD includes
#include <limits>

namespace
{
int basics();
bool readWrite();
bool piecewiseFunctionFromString();
}

//---------------------------------------------------------------------------
int vtkMRMLVolumePropertyNodeTest1(int , char * [] )
{
  bool res = true;
  res = (basics() == EXIT_SUCCESS) && res;
  res = readWrite() && res;
  res = piecewiseFunctionFromString() && res;

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

namespace
{

//---------------------------------------------------------------------------
int basics()
{
  vtkSmartPointer< vtkMRMLVolumePropertyNode > node1 = vtkSmartPointer< vtkMRMLVolumePropertyNode >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  EXERCISE_BASIC_STORABLE_MRML_METHODS(vtkMRMLVolumePropertyNode, node1);
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool readWrite()
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
      scene2->GetNthNodeByClass(0, "vtkMRMLVolumePropertyNode"));
  if (!propertyNode2)
    {
    std::cout << __FUNCTION__ << ":" << __LINE__ << " failed:" << std::endl
              << "  No volume property node found in the loaded scene."
              << std::endl;
    return false;
    }
  vtkPiecewiseFunction* scalarOpacity2 =
    propertyNode2->GetScalarOpacity();
  if (!propertyNode2)
    {
    std::cout << __FUNCTION__ << ":" << __LINE__ << " failed:" << std::endl
              << "  No scalar opacity found in the loaded volume property node."
              << std::endl;
    return false;
    }
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
        return false;
        }
    }
  return true;
}

//---------------------------------------------------------------------------
bool piecewiseFunctionFromString()
{
  std::string s("10 0 0 4.94065645841247e-324 0 69.5504608154297"
                " 0 154.266067504883 0.699999988079071 228 1");
  double expectedData[10] = {0, 0, 4.94065645841247e-324, 0, 69.5504608154297,
                             0, 154.266067504883, 0.699999988079071, 228, 1};
  vtkSmartPointer<vtkPiecewiseFunction> function =
    vtkSmartPointer<vtkPiecewiseFunction>::New();
  vtkMRMLVolumePropertyNode::GetPiecewiseFunctionFromString(s, function);

  if (function->GetSize() != 5)
    {
    std::cout << "Failed to parse string " << s << ", "
              << "found " << function->GetSize() << " values instead of 10";
    return false;
    }
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
      return false;
      }
    }
  return true;
}

}
