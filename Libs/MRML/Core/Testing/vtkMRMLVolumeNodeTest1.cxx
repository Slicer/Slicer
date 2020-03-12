/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeNode.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

// STD includes
#include <list>

//----------------------------------------------------------------------------
class vtkMRMLVolumeNodeTestHelper1 : public vtkMRMLVolumeNode
{
public:
  // Provide a concrete New.
  static vtkMRMLVolumeNodeTestHelper1 *New();

  vtkTypeMacro(vtkMRMLVolumeNodeTestHelper1,vtkMRMLVolumeNode);

  vtkMRMLNode* CreateNodeInstance() override
    {
    return vtkMRMLVolumeNodeTestHelper1::New();
    }
  const char* GetNodeTagName() override
    {
    return "vtkMRMLVolumeNodeTestHelper1";
    }

  vtkMRMLStorageNode* CreateDefaultStorageNode() override { return vtkMRMLVolumeArchetypeStorageNode::New(); }
};
vtkStandardNewMacro(vtkMRMLVolumeNodeTestHelper1);

//----------------------------------------------------------------------------
int vtkMRMLVolumeNodeTest1(int , char * [])
{
  vtkNew<vtkMRMLVolumeNodeTestHelper1> node1;
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  vtkNew<vtkMRMLVolumeNodeTestHelper1> node2;

  node1->CopyOrientation(node2.GetPointer());

  // Scan Order
  vtkNew<vtkMatrix4x4> ijkToRAS;
  const char *scanOrder = node1->ComputeScanOrderFromIJKToRAS(ijkToRAS.GetPointer());
  if (!scanOrder || strcmp(scanOrder, "") == 0)
    {
    std::cerr << "Failed to compute scan order from identity matrix: '" << (scanOrder ? scanOrder : "null") << "'" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Computed scan order from identity matrix: " << (scanOrder ? scanOrder : "null") << std::endl;

  // IJKToRAS <-> Scan Order
  double spacing[3] = {1.0, 1.0, 1.0};
  int dimensions[3] = {2, 2, 2};
  std::vector<std::string> scanOrders;
  scanOrders.emplace_back("IS");
  scanOrders.emplace_back("SI");
  scanOrders.emplace_back("RL");
  scanOrders.emplace_back("LR");
  scanOrders.emplace_back("PA");
  scanOrders.emplace_back("AP");
  for(std::vector<std::string>::iterator it = scanOrders.begin();
      it != scanOrders.end();
      ++it)
    {
    ijkToRAS->Identity();
    vtkMRMLVolumeNode::ComputeIJKToRASFromScanOrder(
          (*it).c_str(), spacing, dimensions, /* centerImage= */ false,
          ijkToRAS.GetPointer());

    const char* computedScanOrder =
        vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(ijkToRAS.GetPointer());

    if (!computedScanOrder ||
        strcmp(computedScanOrder, "") == 0 ||
        *it != computedScanOrder)
      {
      std::cerr << "Failed to compute scan order from '" << *it << "' IJKToRAS matrix: '"
                << (computedScanOrder ? computedScanOrder : "null") << "'"
                << std::endl;
      return EXIT_FAILURE;
      }
    }

  // IJKToRASDirections
  double dirs[3][3];
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      dirs[i][j] = 0.0;
  dirs[0][0] = 1;
  dirs[1][1] = 1;
  dirs[2][2] = 1;
  node1->SetIJKToRASDirections(dirs);
  double getDirs[3][3];
  node1->GetIJKToRASDirections(getDirs);
  for (int i = 0; i < 3; i++)
    {
    for (int j = 0; j < 3; j++)
      {
      if (dirs[i][j] != getDirs[i][j])
        {
        std::cerr << "GetIJKToRASDirections at " << i << ", " << j << ": " << getDirs[i][j] << " not equal to set value: " << dirs[i][j] << std::endl;
        return EXIT_FAILURE;
        }
      }
    }
  std::cout << "IJKToRASDirections passed" << std::endl;

  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, IToRASDirection, 10.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, JToRASDirection, 5.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANDOM(node1, KToRASDirection, 25.0);

  node1->SetIJKToRASDirections(-0.03164, -0.0606374, 5.92996,
                               -0.448103, 0.00428152, -0.418707,
                               0.0, -0.445087, -0.811908);

  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, Spacing, 0.0, 10.0);
  TEST_SET_GET_VECTOR3_DOUBLE_RANGE(node1, Origin, -100.0, 100.0);

  // Origin and Spacing
  double minSpacing = node1->GetMinSpacing();
  std::cout << "Got min spacing = " << minSpacing << std::endl;
  double maxSpacing = node1->GetMaxSpacing();
  std::cout << "Got max spacing = " << maxSpacing << std::endl;

  // matrices
  // IJK to RAS
  ijkToRAS->Identity();
  node1->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  vtkIndent indent;
  std::cout << "IJK to RAS matrix: " << std::endl;
  ijkToRAS->PrintSelf(std::cout, indent.GetNextIndent());

  // RAS to IJK
  vtkNew<vtkMatrix4x4> rasToIJK;
  node1->GetRASToIJKMatrix(rasToIJK.GetPointer());
  std::cout << "RAS to IJK matrix: " << std::endl;
  rasToIJK->PrintSelf(std::cout, indent.GetNextIndent());

  // IJK to RAS direction matrix
  vtkNew<vtkMatrix4x4> ijkToRASDir;
  vtkNew<vtkMatrix4x4> retIJKToRASDir;
  ijkToRASDir->Identity();
  ijkToRASDir->SetElement(0,0,-0.03);
  ijkToRASDir->SetElement(0,1,-0.06);
  ijkToRASDir->SetElement(0,2,5.92);
  ijkToRASDir->SetElement(1,0,-0.44);
  ijkToRASDir->SetElement(1,1,0.004);
  ijkToRASDir->SetElement(1,2,-0.4);
  ijkToRASDir->SetElement(2,1,-0.44);
  ijkToRASDir->SetElement(2,2,-0.8);
  node1->SetIJKToRASDirectionMatrix(ijkToRASDir.GetPointer());
  node1->GetIJKToRASDirectionMatrix(retIJKToRASDir.GetPointer());
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      if (retIJKToRASDir->GetElement(i,j) != ijkToRASDir->GetElement(i,j))
        {
        std::cerr << "Error setting and getting IJKToRASDirectionMatrix, element " << i << ", " << j << " does not match: " << retIJKToRASDir->GetElement(i,j) << " != " << ijkToRASDir->GetElement(i,j) << ". Expected matrix = " << std::endl;
        ijkToRASDir->PrintSelf(std::cerr, indent.GetNextIndent());
        std::cerr << "Returned matrix: " << std::endl;
        retIJKToRASDir->PrintSelf(std::cerr, indent.GetNextIndent());
        return EXIT_FAILURE;
        }
      }
    }

  std::cout << "IJK to RAS direction matrix: " << std::endl;
  retIJKToRASDir->PrintSelf(std::cout, indent.GetNextIndent());

  // Image Data
  vtkNew<vtkImageData> imageData;
  node1->SetAndObserveImageData(nullptr);
  if (node1->GetImageData() != nullptr)
    {
    std::cerr << "Get image data didn't return null" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetAndObserveImageData(imageData.GetPointer());
  vtkImageData *retImageData = node1->GetImageData();
  if (!retImageData)
    {
    std::cerr << "Get image data returned null" << std::endl;
    return EXIT_FAILURE;
    }

  // test some point conversions with known matrices
  double ras[4] = {1.0, 1.0, 1.0, 1.0};
  double ijk[4] = {0.0, 0.0, 0.0, 1.0};
  double ras2[4] = {0.0, 0.0, 0.0, 1.0};
  rasToIJK->Identity();
  node1->SetRASToIJKMatrix(rasToIJK.GetPointer());
  vtkNew<vtkMatrix4x4> retRASToIJK;
  node1->GetRASToIJKMatrix(retRASToIJK.GetPointer());
  retRASToIJK->MultiplyPoint(ras, ijk);
  std::cout << "RAS: [" << ras[0] << "," << ras[1] << "," << ras[2] << "] -> IJK: [" << ijk[0] << "," << ijk[1] << "," << ijk[2] << "]" << std::endl;

  // and back to RAS
  node1->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  std::cout << "IJK to RAS matrix: " << std::endl;
  ijkToRAS->PrintSelf(std::cout, indent.GetNextIndent());
  ijkToRAS->MultiplyPoint(ijk, ras2);
  std::cout << "IJK: [" << ijk[0] << "," << ijk[1] << "," << ijk[2] << "] -> RAS: [" << ras2[0] << "," << ras2[1] << "," << ras2[2] << "]" << std::endl;

  double rasDif = sqrt(vtkMath::Distance2BetweenPoints(ras, ras2));
  std::cout << "\tDiff between RAS input and output via ijk = " << rasDif << std::endl;
  if (rasDif > 0.1)
    {
    std::cout << "ERROR: difference between RAS input and output is " << rasDif << std::endl;
    return EXIT_FAILURE;
    }

  // more complicated
  rasToIJK->SetElement(0,0,-0.03164);
  rasToIJK->SetElement(0,1,-0.0606374);
  rasToIJK->SetElement(0,2,5.92996);
  rasToIJK->SetElement(0,3,-39.3579);
  rasToIJK->SetElement(1,0,-0.448103);
  rasToIJK->SetElement(1,1,0.00428152);
  rasToIJK->SetElement(1,2,-0.418707);
  rasToIJK->SetElement(1,3,161.219);
  rasToIJK->SetElement(2,0,2.18662e-10);
  rasToIJK->SetElement(2,1,-0.445087);
  rasToIJK->SetElement(2,2,-0.811908);
  rasToIJK->SetElement(2,3,118.057);
  node1->SetRASToIJKMatrix(rasToIJK.GetPointer());
  node1->GetRASToIJKMatrix(retRASToIJK.GetPointer());
  std::cout << "Using RAS to IJK matrix: "<< std::endl;
  retRASToIJK->PrintSelf(std::cout, indent.GetNextIndent());
  ras[0] = -11.9901 ;
  ras[1] = -10.6257;
  ras[2] = -1.66493;
  retRASToIJK->MultiplyPoint(ras, ijk);
  std::cout << "RAS: [" << ras[0] << "," << ras[1] << "," << ras[2] << "] -> IJK: [" << ijk[0] << "," << ijk[1] << "," << ijk[2] << "]" << std::endl;
  node1->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  ijkToRAS->MultiplyPoint(ijk, ras2);
  std::cout << "IJK: [" << ijk[0] << "," << ijk[1] << "," << ijk[2] << "] -> RAS: [" << ras[0] << "," << ras[1] << "," << ras[2] << "]" << std::endl;
  rasDif = sqrt(vtkMath::Distance2BetweenPoints(ras, ras2));
  if (rasDif > 0.1)
    {
    std::cout << "ERROR: difference between RAS input and output is " << rasDif << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "\tDiff between RAS input and output via ijk = " << rasDif << std::endl;

  return EXIT_SUCCESS;
}
