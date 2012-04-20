/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// return a concrete storage node, vtkMRMLStorageNode::New returns null
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeNode.h"

#include <vtkPolyData.h>
#include <vtkImageData.h>

#include "vtkMRMLCoreTestingMacros.h"


class vtkMRMLVolumeNodeTestHelper1 : public vtkMRMLVolumeNode
{
public:
  // Provide a concrete New.
  static vtkMRMLVolumeNodeTestHelper1 *New(){return new vtkMRMLVolumeNodeTestHelper1;};

  vtkTypeMacro( vtkMRMLVolumeNodeTestHelper1,vtkMRMLVolumeNode);

  virtual vtkMRMLNode* CreateNodeInstance()
    {
    return new vtkMRMLVolumeNodeTestHelper1;
    }
  virtual const char* GetNodeTagName()
    {
    return "vtkMRMLVolumeNodeTestHelper1";
    }

  double *GetIToRASDirection()
    {
    double dir[3];
    vtkMRMLVolumeNode::GetIToRASDirection(dir);
    double *retdir = dir;
    return retdir;
    }
  double *GetJToRASDirection()
    {
    double dir[3];
    vtkMRMLVolumeNode::GetJToRASDirection(dir);
    double *retdir = dir;
    return retdir;
    }
  double *GetKToRASDirection()
    {
    double dir[3];
    vtkMRMLVolumeNode::GetKToRASDirection(dir);
    double *retdir = dir;
    return retdir;
    }

  virtual vtkMRMLStorageNode* CreateDefaultStorageNode() { return vtkMRMLVolumeArchetypeStorageNode::New(); }
};
 
int vtkMRMLVolumeNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLVolumeNodeTestHelper1 > node1 = vtkSmartPointer< vtkMRMLVolumeNodeTestHelper1 >::New();

  EXERCISE_BASIC_OBJECT_METHODS( node1 );

  EXERCISE_BASIC_DISPLAYABLE_MRML_METHODS(vtkMRMLVolumeNodeTestHelper1, node1);

  vtkSmartPointer< vtkMRMLVolumeNodeTestHelper1 > node2 = vtkSmartPointer< vtkMRMLVolumeNodeTestHelper1 >::New();

  node1->CopyOrientation(node2);

  // Scan Order
  vtkSmartPointer<vtkMatrix4x4> ijkToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  const char *scanOrder = node1->ComputeScanOrderFromIJKToRAS(ijkToRAS);
  if (!scanOrder || strcmp(scanOrder, "") == 0)
    {
    std::cerr << "Failed to compute scan order from identity matrix: '" << (scanOrder ? scanOrder : "null") << "'" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Computed scan order from identity matrix: " << (scanOrder ? scanOrder : "null") << std::endl;

  // IJKToRASDirections
  double dirs[3][3];
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      dirs[i][j] = 0.0;
  dirs[0][0] = 1;
  dirs[1][1] = 1;
  dirs[1][1] = 1;
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
  node1->GetIJKToRASMatrix(ijkToRAS);
  vtkIndent indent;
  std::cout << "IJK to RAS matrix: " << std::endl;
  ijkToRAS->PrintSelf(std::cout, indent.GetNextIndent());

  // RAS to IJK
  vtkSmartPointer<vtkMatrix4x4> rasToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
  node1->GetRASToIJKMatrix(rasToIJK);
  std::cout << "RAS to IJK matrix: " << std::endl;
  rasToIJK->PrintSelf(std::cout, indent.GetNextIndent());

  // IJK to RAS direction matrix
  vtkSmartPointer<vtkMatrix4x4> ijkToRASDir = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> retIJKToRASDir = vtkSmartPointer<vtkMatrix4x4>::New();
  ijkToRASDir->Identity();
  ijkToRASDir->SetElement(0,0,-0.03);
  ijkToRASDir->SetElement(0,1,-0.06);
  ijkToRASDir->SetElement(0,2,5.92);
  ijkToRASDir->SetElement(1,0,-0.44);
  ijkToRASDir->SetElement(1,1,0.004);
  ijkToRASDir->SetElement(1,2,-0.4);
  ijkToRASDir->SetElement(2,1,-0.44);
  ijkToRASDir->SetElement(2,2,-0.8);
  node1->SetIJKToRASDirectionMatrix(ijkToRASDir);
  node1->GetIJKToRASDirectionMatrix(retIJKToRASDir);
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
  vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
  node1->SetAndObserveImageData(NULL);
  if (node1->GetImageData() != NULL)
    {
    std::cerr << "Get image data didn't return null" << std::endl;
    return EXIT_FAILURE;
    }
  node1->SetAndObserveImageData(imageData);
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
  node1->SetRASToIJKMatrix(rasToIJK);
  vtkSmartPointer<vtkMatrix4x4> retRASToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
  node1->GetRASToIJKMatrix(retRASToIJK);
  retRASToIJK->MultiplyPoint(ras, ijk);
  std::cout << "RAS: [" << ras[0] << "," << ras[1] << "," << ras[2] << "] -> IJK: [" << ijk[0] << "," << ijk[1] << "," << ijk[2] << "]" << std::endl;

  // and back to RAS
  node1->GetIJKToRASMatrix(ijkToRAS);
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
  node1->SetRASToIJKMatrix(rasToIJK);
  node1->GetRASToIJKMatrix(retRASToIJK);
  std::cout << "Using RAS to IJK matrix: "<< std::endl;
  retRASToIJK->PrintSelf(std::cout, indent.GetNextIndent());
  ras[0] = -11.9901 ;
  ras[1] = -10.6257;
  ras[2] = -1.66493;
  retRASToIJK->MultiplyPoint(ras, ijk);
  std::cout << "RAS: [" << ras[0] << "," << ras[1] << "," << ras[2] << "] -> IJK: [" << ijk[0] << "," << ijk[1] << "," << ijk[2] << "]" << std::endl;
  node1->GetIJKToRASMatrix(ijkToRAS);
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
