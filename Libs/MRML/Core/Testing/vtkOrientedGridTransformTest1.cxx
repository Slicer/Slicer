/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkOrientedGridTransform.h"

// ITK includes
#include <itkAffineTransform.h>
#include <itkDisplacementFieldTransform.h>

// VTK includes
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkNew.h"

typedef double itkVectorComponentType;
typedef itk::Vector<itkVectorComponentType, 3> itkVectorPixelType;
typedef itk::Image<itkVectorPixelType,  3> itkDisplacementFieldType;
typedef itk::DisplacementFieldTransform<itkVectorComponentType,3> itkGridTransformType;


//----------------------------------------------------------------------------
void CreateGridTransformVtk(vtkOrientedGridTransform* gridTransform,
  double origin[3], double spacing[3], double direction[3][3], double dims[3])
{
  vtkNew<vtkImageData> gridCoefficients;
  gridCoefficients->SetExtent(0, dims[0]-1, 0, dims[1]-1, 0, dims[2]-1);
  gridCoefficients->SetOrigin(origin);
  gridCoefficients->SetSpacing(spacing);

  gridCoefficients->AllocateScalars(VTK_DOUBLE, 3);

  // Fill with 0
  for (int z = 0; z < dims[2]; z++)
  {
  for (int y = 0; y < dims[1]; y++)
    {
    for (int x = 0; x < dims[0]; x++)
      {
      gridCoefficients->SetScalarComponentFromDouble(x,y,z,0, 0.0);
      gridCoefficients->SetScalarComponentFromDouble(x,y,z,1, 0.0);
      gridCoefficients->SetScalarComponentFromDouble(x,y,z,2, 0.0);
      }
    }
  }

  vtkNew<vtkMatrix4x4> gridOrientation;
  for (int row=0; row<3; row++)
    {
    for (int col=0; col<3; col++)
      {
      gridOrientation->SetElement(row,col,direction[row][col]);
      }
    }

  gridTransform->SetGridDirectionMatrix(gridOrientation.GetPointer());
  gridTransform->SetDisplacementGridData(gridCoefficients.GetPointer());

  gridTransform->SetInterpolationModeToCubic();
}

//----------------------------------------------------------------------------
void SetGridNodeVtk(vtkOrientedGridTransform* gridTransform,int nodeIndex[3], double nodeValue[3])
{
  vtkImageData* gridCoefficients=gridTransform->GetDisplacementGrid();

  gridCoefficients->SetScalarComponentFromDouble(nodeIndex[0],nodeIndex[1],nodeIndex[2],0, nodeValue[0]);
  gridCoefficients->SetScalarComponentFromDouble(nodeIndex[0],nodeIndex[1],nodeIndex[2],1, nodeValue[1]);
  gridCoefficients->SetScalarComponentFromDouble(nodeIndex[0],nodeIndex[1],nodeIndex[2],2, nodeValue[2]);
}

//----------------------------------------------------------------------------
itkGridTransformType::Pointer CreateGridTransformItk(
  const double origin[3], const double spacing[3], double direction[3][3], const double dims[3])
{
  itkDisplacementFieldType::RegionType region;
  itkDisplacementFieldType::SpacingType spacinItk;
  itkDisplacementFieldType::RegionType::SizeType sz;
  itkDisplacementFieldType::DirectionType directionItk;

  for (int i=0; i<3; i++)
    {
    spacinItk[i] = spacing[i];
    sz[i]=dims[i];
    directionItk(i,0)=direction[i][0];
    directionItk(i,1)=direction[i][1];
    directionItk(i,2)=direction[i][2];
    }

  region.SetSize( sz );

  itkGridTransformType::RegionType::IndexType idx;
  idx[0] = idx[1] = idx[2] = 0;
  region.SetIndex( idx );

  itkDisplacementFieldType::Pointer displacementField = itkDisplacementFieldType::New();

  displacementField->SetRegions( region );
  displacementField->SetOrigin( origin );
  displacementField->SetSpacing( spacinItk );
  displacementField->SetDirection( directionItk );
  displacementField->Allocate();
  itkVectorPixelType zeroDisplacement;
  zeroDisplacement.Fill(0.0);
  displacementField->FillBuffer(zeroDisplacement);

  itkGridTransformType::Pointer grid = itkGridTransformType::New();
  grid->SetDisplacementField(displacementField.GetPointer());

  return grid;
}

//----------------------------------------------------------------------------
void SetGridNodeItk(itkGridTransformType::Pointer grid,int nodeIndex[3], double nodeValue[3])
{
  itkDisplacementFieldType::Pointer displacementField = grid->GetDisplacementField();

  itkGridTransformType::IndexType pixelIndex;
  pixelIndex[0] = nodeIndex[0];
  pixelIndex[1] = nodeIndex[1];
  pixelIndex[2] = nodeIndex[2];

  itkVectorPixelType pixelValue;
  pixelValue[0]=nodeValue[0];
  pixelValue[1]=nodeValue[1];
  pixelValue[2]=nodeValue[2];

  displacementField->SetPixel(pixelIndex, pixelValue);
}

//----------------------------------------------------------------------------
// Compute transformed point differences between ITK and VTK grid transform implementations
double getTransformedPointDifferenceItkVtk(const double inputPoint[3], itkGridTransformType::Pointer gridItk, vtkOrientedGridTransform* gridVtk, bool logDetails)
{
  // ITK
  itkGridTransformType::InputPointType inputPointItk;
  inputPointItk[0] = inputPoint[0];
  inputPointItk[1] = inputPoint[1];
  inputPointItk[2] = inputPoint[2];
  itkGridTransformType::OutputPointType outputPointItk;
  outputPointItk = gridItk->TransformPoint( inputPointItk );

  // VTK
  double outputPoint[3]={0};
  gridVtk->TransformPoint( inputPoint, outputPoint );

  itk::Point<double,3> inputPointVtk( inputPoint );
  itk::Point<double,3> outputPointVtk( outputPoint );
  double difference = outputPointItk.EuclideanDistanceTo( outputPointVtk );

  if (logDetails)
    {
    std::cout << "Compare ITK and VTK transform results" << std::endl;
    std::cout << " Input point: " << inputPoint[0] << " " << inputPoint[1] << " " << inputPoint[2] << std::endl;
    std::cout << " Output point (transformed by ITK): " << outputPointItk << std::endl;
    std::cout << " Output point (transformed by VTK): " << outputPoint[0] << " " << outputPoint[1] << " " << outputPoint[2] << std::endl;
    std::cout << " Difference between ITK and VTK transform results: " << difference << std::endl;
    }

  return difference;
}

//----------------------------------------------------------------------------
// Compute transformed point differences between single-precision and double-precision VTK grid transform implementations
double getTransformedPointDifferenceSingleDoubleVtk(const double inputPoint[3], vtkOrientedGridTransform* gridVtk, bool logDetails)
{
  double outputPoint[3]={0};
  gridVtk->TransformPoint( inputPoint, outputPoint );

  float floatInputPoint[3]={0};
  floatInputPoint[0]=static_cast<float>(inputPoint[0]);
  floatInputPoint[1]=static_cast<float>(inputPoint[1]);
  floatInputPoint[2]=static_cast<float>(inputPoint[2]);
  float floatOutputPoint[3]={0};
  gridVtk->TransformPoint( floatInputPoint, floatOutputPoint );

  itk::Point<double,3> outputPointVtk( outputPoint );
  itk::Point<double,3> floatOutputPointVtk;
  floatOutputPointVtk[0] = floatOutputPoint[0];
  floatOutputPointVtk[1] = floatOutputPoint[1];
  floatOutputPointVtk[2] = floatOutputPoint[2];
  double difference = outputPointVtk.EuclideanDistanceTo( floatOutputPointVtk );

  if (logDetails)
    {
    std::cout << "Compare single-precision and double-precision VTK transform results" << std::endl;
    std::cout << " Input point: " << inputPoint[0] << " " << inputPoint[1] << " " << inputPoint[2] << std::endl;
    std::cout << " Output point (transformed by VTK, double-precision): " << outputPoint[0] << " " << outputPoint[1] << " " << outputPoint[2] << std::endl;
    std::cout << " Output point (transformed by VTK, single-precision): " << floatOutputPoint[0] << " " << floatOutputPoint[1] << " " << floatOutputPoint[2] << std::endl;
    std::cout << " Difference between single-precision and double-precision VTK transform results: " << difference << std::endl;
    }

  return difference;
}

//----------------------------------------------------------------------------
// Compute the error of derivative computation in VTK grid transform implementation
double getDerivativeErrorVtk(const double inputPoint[3], vtkOrientedGridTransform* gridVtk, bool logDetails)
{
  // Jacobian estimated using central difference
  double jacobianEstimation[3][3];
  double eps=1e-1; // step size
  for (int row=0; row<3; row++)
    {
    double xMinus1[3]={inputPoint[0],inputPoint[1],inputPoint[2]};
    double xPlus1[3]={inputPoint[0],inputPoint[1],inputPoint[2]};
    xMinus1[row]-=eps;
    xPlus1[row]+=eps;
    double xMinus1Transformed[3]={0};
    gridVtk->TransformPoint( xMinus1, xMinus1Transformed);
    double xPlus1Transformed[3]={0};
    gridVtk->TransformPoint( xPlus1, xPlus1Transformed);
    for (int col=0; col<3; col++)
      {
      jacobianEstimation[col][row] = (xPlus1Transformed[col]-xMinus1Transformed[col])/(2*eps);
      }
    }

  // Jacobian computed by the transform class
  double outputPoint[3]={0};
  double jacobianVtk[3][3];
  gridVtk->InternalTransformDerivative( inputPoint, outputPoint, jacobianVtk );

  if (logDetails)
    {
    std::cout << "Compare VTK transform Jacobian to an estimation" << std::endl;
    std::cout << " Input point: " << inputPoint[0] << " " << inputPoint[1] << " " << inputPoint[2] << std::endl;
    }

  double maxDifference = 0;
  for (int row=0; row<3; row++)
    {
    for (int col=0; col<3; col++)
      {
      double difference=fabs(jacobianVtk[row][col]-jacobianEstimation[row][col]);
      if (difference>maxDifference)
        {
        maxDifference = difference;
        }
      if (logDetails)
        {
        std::cout << " Element[" << row << "," << col << "]: Ground truth: "<< jacobianEstimation[row][col] << "  VTK: " << jacobianVtk[row][col]
          << "  Difference: " << difference << std::endl;
        }
      }
    }

  return maxDifference;
}

//----------------------------------------------------------------------------
// Compute the error of inverse computation in VTK grid transform implementation
double getInverseErrorVtk(const double inputPoint[3], vtkOrientedGridTransform* gridVtk, bool logDetails)
{
  double outputPoint[3] = { -1, -1, -1 };
  gridVtk->TransformPoint( inputPoint, outputPoint);

  double inversePoint[3] = { -1, -1, -1 };
  gridVtk->Inverse();
  gridVtk->TransformPoint( outputPoint, inversePoint );
  gridVtk->Inverse();

  itk::Point<double,3> inputPointVtk( inputPoint );
  itk::Point<double,3> inversePointVtk( inversePoint );
  double errorOfInverseComputation = inputPointVtk.EuclideanDistanceTo( inversePointVtk );

  if (logDetails)
    {
    std::cout << "Verify VTK transform inverse" << std::endl;
    std::cout << " Input point: " << inputPoint[0] << " " << inputPoint[1] << " " << inputPoint[2] << std::endl;
    std::cout << " Transformed point: " << outputPoint[0] << " " << outputPoint[1] << " " << outputPoint[2] << std::endl;
    std::cout << " Transformed point transformed by inverse: " << inversePoint[0] << " " << inversePoint[1] << " " << inversePoint[2] << std::endl;
    std::cout << " Difference between VTK transform inverse and the ground truth: " << errorOfInverseComputation << std::endl;
    }

  return errorOfInverseComputation;
}

//----------------------------------------------------------------------------
int vtkOrientedGridTransformTest1(int , char * [] )
{
  double averageSpacing=100;
  double origin[3] = {-100, -100, -100};
  double spacing[3] = {averageSpacing, averageSpacing, averageSpacing};
  double direction[3][3] = {{0.92128500, -0.36017075, -0.146666625}, {0.31722386, 0.91417248, -0.25230478}, {0.22495105, 0.18591857, 0.95646814}};
  //double direction[3][3] = {{1,0,0}, {0,1,0}, {0,0,1}};
  double dims[3] = {7,8,7};

  // Modify a few nodes
  int modifiedGridNodeIndex1[3] = {2, 3, 5};
  double modifiedGridNodeValue1[3] = {20.0, -30.0, 60.0};
  int modifiedGridNodeIndex2[3] = {4, 4, 3};
  double modifiedGridNodeValue2[3] = {-20.0, 40.0, 20.0};
  int modifiedGridNodeIndex3[3] = {0, 0, 6};
  double modifiedGridNodeValue3[3] = {50.0, 70.0, -60.0};

  // Create an ITK grid transform. It'll serve as the reference.
  itkGridTransformType::Pointer gridItk=CreateGridTransformItk(origin, spacing, direction, dims);
  // Modify a grid node
  SetGridNodeItk(gridItk, modifiedGridNodeIndex1, modifiedGridNodeValue1);
  SetGridNodeItk(gridItk, modifiedGridNodeIndex2, modifiedGridNodeValue2);
  SetGridNodeItk(gridItk, modifiedGridNodeIndex3, modifiedGridNodeValue3);

  // Create a VTK grid transform with the same parameters.
  vtkNew<vtkOrientedGridTransform> gridVtk;
  CreateGridTransformVtk(gridVtk.GetPointer(), origin, spacing, direction, dims);
  // Modify a grid node
  SetGridNodeVtk(gridVtk.GetPointer(), modifiedGridNodeIndex1, modifiedGridNodeValue1);
  SetGridNodeVtk(gridVtk.GetPointer(), modifiedGridNodeIndex2, modifiedGridNodeValue2);
  SetGridNodeVtk(gridVtk.GetPointer(), modifiedGridNodeIndex3, modifiedGridNodeValue3);

  int numberOfPointsTested=0;
  int numberOfItkVtkPointMismatches=0;
  int numberOfSingleDoubleVtkPointMismatches=0;
  int numberOfDerivativeMismatches=0;
  int numberOfInverseMismatches=0;

  // We take samples in the grid region (first node + 2 < node < last node - 1)
  // because the boundaries are handled differently in ITK and VTK (in ITK there is an
  // abrupt change to 0, while in VTK it is smoothly converges to zero)
  const int numberOfSamplesPerAxis=25;
  const double startMargin=0.5;
  const double endMargin=0.5;
  const double startI = startMargin;
  const double endI = (dims[0]-1)-endMargin;
  const double startJ = startMargin;
  const double endJ = (dims[1]-1)-endMargin;
  const double startK = startMargin;
  const double endK = (dims[2]-1)-endMargin;
  const double incI=(endI-startI)/numberOfSamplesPerAxis;
  const double incJ=(endJ-startJ)/numberOfSamplesPerAxis;
  const double incK=(endK-startK)/numberOfSamplesPerAxis;

  // The default ITK interpolator is linear, compare the results to that
  gridVtk->SetInterpolationModeToLinear();
  for (double k=startK+incK; k<=endK-incK; k+=incK)
    {
    for (double j=startJ+incJ; j<=endJ-incJ; j+=incJ)
      {
      for (double i=startI+incI; i<=endI-incI; i+=incI)
        {
        numberOfPointsTested++;
        double inputPoint[3];
        inputPoint[0] = origin[0]+direction[0][0]*spacing[0]*i+direction[0][1]*spacing[1]*j+direction[0][2]*spacing[2]*k;
        inputPoint[1] = origin[1]+direction[1][0]*spacing[0]*i+direction[1][1]*spacing[1]*j+direction[1][2]*spacing[2]*k;
        inputPoint[2] = origin[2]+direction[2][0]*spacing[0]*i+direction[2][1]*spacing[1]*j+direction[2][2]*spacing[2]*k;
        // Compare transformation results computed by ITK and VTK.
        double differenceItkVtk = getTransformedPointDifferenceItkVtk(inputPoint, gridItk, gridVtk.GetPointer(), false);
        if ( differenceItkVtk > 1e-2 )
          {
          getTransformedPointDifferenceItkVtk(inputPoint, gridItk, gridVtk.GetPointer(), true);
          std::cout << "ERROR: Point transform result mismatch between ITK and VTK at grid point ("<<i<<","<<j<<","<<k<<") with linear interpolation"<< std::endl;
          numberOfItkVtkPointMismatches++;
          }
        }
      }
    }

  gridVtk->SetInterpolationModeToCubic();
  for (double k=startK+incK; k<=endK-incK; k+=incK)
    {
    for (double j=startJ+incJ; j<=endJ-incJ; j+=incJ)
      {
      for (double i=startI+incI; i<=endI-incI; i+=incI)
        {
        numberOfPointsTested++;
        double inputPoint[3];
        inputPoint[0] = origin[0]+direction[0][0]*spacing[0]*i+direction[0][1]*spacing[1]*j+direction[0][2]*spacing[2]*k;
        inputPoint[1] = origin[1]+direction[1][0]*spacing[0]*i+direction[1][1]*spacing[1]*j+direction[1][2]*spacing[2]*k;
        inputPoint[2] = origin[2]+direction[2][0]*spacing[0]*i+direction[2][1]*spacing[1]*j+direction[2][2]*spacing[2]*k;
        // Compare transformation results computed by ITK and VTK.
        double differenceItkVtk = getTransformedPointDifferenceItkVtk(inputPoint, gridItk, gridVtk.GetPointer(), false);
        // the larger the distance between the grid points, the larger difference is expected between ITK's linear and VTK's cubic
        // interpolation, therefore make the threshold the 20% of the spacing
        if ( differenceItkVtk > averageSpacing*0.20 )
          {
          getTransformedPointDifferenceItkVtk(inputPoint, gridItk, gridVtk.GetPointer(), true);
          std::cout << "ERROR: Point transform result mismatch between ITK and VTK at grid point ("<<i<<","<<j<<","<<k<<") with cubic interpolation"<< std::endl;
          numberOfItkVtkPointMismatches++;
          }
        // Verify single/double-precision computation difference
        double differenceSingleDoubleVtk = getTransformedPointDifferenceSingleDoubleVtk(inputPoint, gridVtk.GetPointer(), false);
        if ( differenceSingleDoubleVtk > 1e-4 )
          {
          getTransformedPointDifferenceSingleDoubleVtk(inputPoint, gridVtk.GetPointer(), true);
          std::cout << "ERROR: Point transform result mismatch between single-precision and double-precision VTK computation at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfSingleDoubleVtkPointMismatches++;
          }
        // Verify VTK derivative
        double derivativeError = getDerivativeErrorVtk(inputPoint, gridVtk.GetPointer(), false);
        if ( derivativeError > 1e-2 )
          {
          getDerivativeErrorVtk(inputPoint, gridVtk.GetPointer(), true);
          std::cout << "ERROR: Transform derivative result mismatch between VTK and numerical approximation at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfDerivativeMismatches++;
          }
        // Verify VTK inverse transform
        double inverseError = getInverseErrorVtk(inputPoint, gridVtk.GetPointer(), false);
        // add 10% to the inverse tolerance, as the point is transformed twice, so the error can be slightly higher
        // than a single inverse computation
        if ( inverseError > gridVtk->GetInverseTolerance()*1.10 )
          {
          getInverseErrorVtk(inputPoint, gridVtk.GetPointer(), true);
          std::cout << "ERROR: Point transformed by forward and inverse transform does not match the original point" << std::endl;
          numberOfInverseMismatches++;
          }
        }
      }
    }

  std::cout << "Number of points tested: " << numberOfPointsTested << std::endl;
  std::cout << "Number of ITK/VTK mismatches: " << numberOfItkVtkPointMismatches << std::endl;
  std::cout << "Number of single/double precision mismatches: " << numberOfSingleDoubleVtkPointMismatches << std::endl;
  std::cout << "Number of derivative mismatches: " << numberOfDerivativeMismatches << std::endl;
  std::cout << "Number of inverse mismatches: " << numberOfInverseMismatches << std::endl;

  if (numberOfItkVtkPointMismatches==0 && numberOfDerivativeMismatches==0 && numberOfInverseMismatches==0)
    {
    std::cout << "Test result: PASSED" << std::endl;
    return EXIT_SUCCESS;
    }
  else
    {
    std::cout << "Test result: FAILED" << std::endl;
    return EXIT_FAILURE;
    }
}
