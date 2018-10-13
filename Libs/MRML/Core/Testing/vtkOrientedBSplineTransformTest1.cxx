/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkOrientedBSplineTransform.h"

// ITK includes
#include <itkAffineTransform.h>
#include <itkBSplineDeformableTransform.h>

// VTK includes
#include "vtkImageData.h"
#include "vtkMatrix4x4.h"
#include "vtkNew.h"

typedef itk::BSplineDeformableTransform<double,3,3> itkBSplineType;

double DisplacementScale=0.63;

//----------------------------------------------------------------------------
void CreateBSplineVtk(vtkOrientedBSplineTransform* bsplineTransform,
  double origin[3], double spacing[3], double direction[3][3], double dims[3],
  const double bulkMatrix[3][3], const double bulkOffset[3])
{
  vtkNew<vtkImageData> bsplineCoefficients;
  bsplineCoefficients->SetExtent(0, dims[0]-1, 0, dims[1]-1, 0, dims[2]-1);
  bsplineCoefficients->SetOrigin(origin);
  bsplineCoefficients->SetSpacing(spacing);

  bsplineCoefficients->AllocateScalars(VTK_DOUBLE, 3);


  // Fill with 0
  for (int z = 0; z < dims[2]; z++)
  {
  for (int y = 0; y < dims[1]; y++)
    {
    for (int x = 0; x < dims[0]; x++)
      {
      bsplineCoefficients->SetScalarComponentFromDouble(x,y,z,0, 0.0);
      bsplineCoefficients->SetScalarComponentFromDouble(x,y,z,1, 0.0);
      bsplineCoefficients->SetScalarComponentFromDouble(x,y,z,2, 0.0);
      }
    }
  }

  vtkNew<vtkMatrix4x4> bulkTransform;
  vtkNew<vtkMatrix4x4> gridOrientation;
  for (int row=0; row<3; row++)
    {
    for (int col=0; col<3; col++)
      {
      bulkTransform->SetElement(row,col,bulkMatrix[row][col]);
      gridOrientation->SetElement(row,col,direction[row][col]);
      }
    bulkTransform->SetElement(row,3,bulkOffset[row]);
    }

  bsplineTransform->SetGridDirectionMatrix(gridOrientation.GetPointer());
  bsplineTransform->SetCoefficientData(bsplineCoefficients.GetPointer());
  bsplineTransform->SetBulkTransformMatrix(bulkTransform.GetPointer());

  bsplineTransform->SetBorderModeToZero();
  bsplineTransform->SetDisplacementScale(DisplacementScale);
}

//----------------------------------------------------------------------------
void SetBSplineNodeVtk(vtkOrientedBSplineTransform* bsplineTransform,int nodeIndex[3], double nodeValue[3])
{
  vtkImageData* bsplineCoefficients=bsplineTransform->GetCoefficientData();

  bsplineCoefficients->SetScalarComponentFromDouble(nodeIndex[0],nodeIndex[1],nodeIndex[2],0, nodeValue[0]);
  bsplineCoefficients->SetScalarComponentFromDouble(nodeIndex[0],nodeIndex[1],nodeIndex[2],1, nodeValue[1]);
  bsplineCoefficients->SetScalarComponentFromDouble(nodeIndex[0],nodeIndex[1],nodeIndex[2],2, nodeValue[2]);
}

//----------------------------------------------------------------------------
itkBSplineType::Pointer CreateBSplineItk(
  const double origin[3], const double spacing[3], double direction[3][3], const double dims[3],
  const double bulkMatrix[3][3], const double bulkOffset[3])
{
  itkBSplineType::RegionType region;
  itkBSplineType::OriginType originItk;
  itkBSplineType::SpacingType spacinItk;
  itkBSplineType::RegionType::SizeType sz;
  itkBSplineType::DirectionType directionItk;

  for (int i=0; i<3; i++)
    {
    originItk[i] = origin[i];
    spacinItk[i] = spacing[i];
    sz[i]=dims[i];
    directionItk(i,0)=direction[i][0];
    directionItk(i,1)=direction[i][1];
    directionItk(i,2)=direction[i][2];
    }

  region.SetSize( sz );

  itkBSplineType::RegionType::IndexType idx;
  idx[0] = idx[1] = idx[2] = 0;
  region.SetIndex( idx );

  itkBSplineType::Pointer bspline = itkBSplineType::New();
  bspline->SetGridRegion( region );
  bspline->SetGridOrigin( originItk );
  bspline->SetGridSpacing( spacinItk );
  bspline->SetGridDirection( directionItk );

  typedef itk::AffineTransform< double,3 > BulkTransformType;
  const BulkTransformType::Pointer bulkTransform = BulkTransformType::New();
  BulkTransformType::MatrixType m;
  for (int row=0; row<3; row++)
    {
    for (int col=0; col<3; col++)
      {
      m[row][col]=bulkMatrix[row][col];
      }
    }
  bulkTransform->SetMatrix(m);
  BulkTransformType::OffsetType v;
  v[0]=bulkOffset[0];
  v[1]=bulkOffset[1];
  v[2]=bulkOffset[2];
  bulkTransform->SetOffset(v);

  bspline->SetBulkTransform(bulkTransform);

  const unsigned int numberOfParameters = bspline->GetNumberOfParameters();
  typedef itkBSplineType::ParametersType ParametersType;
  ParametersType parameters( numberOfParameters );
  // Check if numberOfNodes = numberOfParameters / 3;
  assert( (numberOfParameters / 3) == dims[0] * dims[1] * dims[2] );
  parameters.Fill( 0.0 );

  bspline->SetParameters( parameters );

  return bspline;
}

//----------------------------------------------------------------------------
void SetBSplineNodeItk(itkBSplineType::Pointer bspline,int nodeIndex[3], double nodeValue[3])
{
  itkBSplineType::RegionType region = bspline->GetGridRegion();
  itkBSplineType::RegionType::SizeType dims = region.GetSize();
  itkBSplineType::ParametersType parameters = bspline->GetParameters();

  const unsigned int numberOfNodes = dims[0] * dims[1] * dims[2];
  parameters.SetElement( numberOfNodes*0 + nodeIndex[0] + nodeIndex[1]*dims[0] + nodeIndex[2]*dims[0]*dims[1], DisplacementScale*nodeValue[0] );
  parameters.SetElement( numberOfNodes*1 + nodeIndex[0] + nodeIndex[1]*dims[0] + nodeIndex[2]*dims[0]*dims[1], DisplacementScale*nodeValue[1] );
  parameters.SetElement( numberOfNodes*2 + nodeIndex[0] + nodeIndex[1]*dims[0] + nodeIndex[2]*dims[0]*dims[1], DisplacementScale*nodeValue[2] );

  bspline->SetParameters( parameters );
}

//----------------------------------------------------------------------------
// Compute transformed point differences between ITK and VTK BSpline implementations
double getTransformedPointDifferenceItkVtk(const double inputPoint[3], itkBSplineType::Pointer bsplineItk, vtkOrientedBSplineTransform* bsplineVtk, bool logDetails)
{
  // ITK
  itkBSplineType::InputPointType inputPointItk;
  inputPointItk[0] = inputPoint[0];
  inputPointItk[1] = inputPoint[1];
  inputPointItk[2] = inputPoint[2];
  itkBSplineType::OutputPointType outputPointItk;
  outputPointItk = bsplineItk->TransformPoint( inputPointItk );

  // VTK
  double outputPoint[3]={0};
  bsplineVtk->TransformPoint( inputPoint, outputPoint );

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
// Compute transformed point differences between single-precision and double-precision VTK BSpline implementations
double getTransformedPointDifferenceSingleDoubleVtk(const double inputPoint[3], vtkOrientedBSplineTransform* bsplineVtk, bool logDetails)
{
  double outputPoint[3]={0};
  bsplineVtk->TransformPoint( inputPoint, outputPoint );

  float floatInputPoint[3]={0};
  floatInputPoint[0]=static_cast<float>(inputPoint[0]);
  floatInputPoint[1]=static_cast<float>(inputPoint[1]);
  floatInputPoint[2]=static_cast<float>(inputPoint[2]);
  float floatOutputPoint[3]={0};
  bsplineVtk->TransformPoint( floatInputPoint, floatOutputPoint );

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
// Compute the error of derivative computation in VTK BSpline implementation
double getDerivativeErrorVtk(const double inputPoint[3], vtkOrientedBSplineTransform* bsplineVtk, bool logDetails)
{
  // Jacobian estimated using central difference
  double jacobianEstimation[3][3];
  double eps=1e-3; // step size
  for (int row=0; row<3; row++)
    {
    double xMinus1[3]={inputPoint[0],inputPoint[1],inputPoint[2]};
    double xPlus1[3]={inputPoint[0],inputPoint[1],inputPoint[2]};
    xMinus1[row]-=eps;
    xPlus1[row]+=eps;
    double xMinus1Transformed[3]={0};
    bsplineVtk->TransformPoint( xMinus1, xMinus1Transformed);
    double xPlus1Transformed[3]={0};
    bsplineVtk->TransformPoint( xPlus1, xPlus1Transformed);
    for (int col=0; col<3; col++)
      {
      jacobianEstimation[col][row] = (xPlus1Transformed[col]-xMinus1Transformed[col])/(2*eps);
      }
    }

  // Jacobian computed by the transform class
  double outputPoint[3]={0};
  double jacobianVtk[3][3];
  bsplineVtk->InternalTransformDerivative( inputPoint, outputPoint, jacobianVtk );

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
// Compute the error of inverse computation in VTK BSpline implementation
double getInverseErrorVtk(const double inputPoint[3], vtkOrientedBSplineTransform* bsplineVtk, bool logDetails)
{
  double outputPoint[3] = { -1, -1, -1 };
  bsplineVtk->TransformPoint( inputPoint, outputPoint);

  double inversePoint[3] = { -1, -1, -1 };
  bsplineVtk->Inverse();
  bsplineVtk->TransformPoint( outputPoint, inversePoint );
  bsplineVtk->Inverse();

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
int vtkOrientedBSplineTransformTest1(int , char * [] )
{
  // we want to transform a 300x400x300 image, with grid points 100
  // pixels apart. So, we need 4x5x4 "interior" grid points. Since the
  // spline order is 3, we need 1 exterior node on the "left" and 2
  // exterior nodes on the "right".  So, we need a spline grid of
  // 7x8x7. We want to transform the whole image, so the origin should
  // be set to spline grid location (0,0).


  //  B--o--o--o--o--o--o   A = image(0,0)       A = spline_grid(1,1)
  //  |  |  |  |  |  |  |   B = image(-100,-100) B = spline_grid(0,0)
  //  o--A--+--+--+--o--o
  //  |  |  |  |  |  |  |
  //  o--+--+--+--+--o--o   origin should be set to (-100,-100)
  //  |  |  |  |  |  |  |
  //  o--+--+--+--+--o--o
  //  |  |  |  |  |  |  |
  //  o--o--o--o--o--o--o
  //  |  |  |  |  |  |  |
  //  o--o--o--o--o--o--o

  double origin[3] = {-100, -100, -100};
  double spacing[3] = {100, 100, 100};
  double direction[3][3] = {{0.92128500, -0.36017075, -0.146666625}, {0.31722386, 0.91417248, -0.25230478}, {0.22495105, 0.18591857, 0.95646814}};
  double dims[3] = {7,8,7};

  const double bulkMatrix[3][3]={ { 0.7, 0.2, 0.1 }, { 0.1, 0.8, 0.1 }, { 0.05, 0.2, 0.9 }};
  const double bulkOffset[3]={-5, 3, 6};

  // Modify a few nodes
  int modifiedBSplineNodeIndex1[3] = {2, 3, 5};
  double modifiedBSplineNodeValue1[3] = {20.0, -30.0, 60.0};
  int modifiedBSplineNodeIndex2[3] = {4, 4, 3};
  double modifiedBSplineNodeValue2[3] = {-20.0, 40.0, 20.0};
  int modifiedBSplineNodeIndex3[3] = {0, 0, 6};
  double modifiedBSplineNodeValue3[3] = {50.0, 70.0, -60.0};

  // Create an ITK BSpline transform. It'll serve as the reference.
  itkBSplineType::Pointer bsplineItk=CreateBSplineItk(origin, spacing, direction, dims, bulkMatrix, bulkOffset);
  // Modify a BSpline node
  SetBSplineNodeItk(bsplineItk, modifiedBSplineNodeIndex1, modifiedBSplineNodeValue1);
  SetBSplineNodeItk(bsplineItk, modifiedBSplineNodeIndex2, modifiedBSplineNodeValue2);
  SetBSplineNodeItk(bsplineItk, modifiedBSplineNodeIndex3, modifiedBSplineNodeValue3);

  // Create a VTK BSpline transform with the same parameters.
  vtkNew<vtkOrientedBSplineTransform> bsplineVtk;
  CreateBSplineVtk(bsplineVtk.GetPointer(), origin, spacing, direction, dims, bulkMatrix, bulkOffset);
  // Modify a BSpline node
  SetBSplineNodeVtk(bsplineVtk.GetPointer(), modifiedBSplineNodeIndex1, modifiedBSplineNodeValue1);
  SetBSplineNodeVtk(bsplineVtk.GetPointer(), modifiedBSplineNodeIndex2, modifiedBSplineNodeValue2);
  SetBSplineNodeVtk(bsplineVtk.GetPointer(), modifiedBSplineNodeIndex3, modifiedBSplineNodeValue3);

  int numberOfPointsTested=0;
  int numberOfItkVtkPointMismatches=0;
  int numberOfSingleDoubleVtkPointMismatches=0;
  int numberOfDerivativeMismatches=0;
  int numberOfInverseMismatches=0;

  // We take samples in the bspline region (first node + 2 < node < last node - 1)
  // because the boundaries are handled differently in ITK and VTK (in ITK there is an
  // abrupt change to 0, while in VTK it is smoothly converges to zero)
  const int numberOfSamplesPerAxis=25;
  const double startMargin=2;
  const double endMargin=1;
  const double startI = startMargin;
  const double endI = (dims[0]-1)-endMargin;
  const double startJ = startMargin;
  const double endJ = (dims[1]-1)-endMargin;
  const double startK = startMargin;
  const double endK = (dims[2]-1)-endMargin;
  const double incI=(endI-startI)/numberOfSamplesPerAxis;
  const double incJ=(endJ-startJ)/numberOfSamplesPerAxis;
  const double incK=(endK-startK)/numberOfSamplesPerAxis;
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
        double differenceItkVtk = getTransformedPointDifferenceItkVtk(inputPoint, bsplineItk, bsplineVtk.GetPointer(), false);
        if ( differenceItkVtk > 1e-6 )
          {
          getTransformedPointDifferenceItkVtk(inputPoint, bsplineItk, bsplineVtk.GetPointer(), true);
          std::cout << "ERROR: Point transform result mismatch between ITK and VTK at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfItkVtkPointMismatches++;
          }
        double differenceSingleDoubleVtk = getTransformedPointDifferenceSingleDoubleVtk(inputPoint, bsplineVtk.GetPointer(), false);
        if ( differenceSingleDoubleVtk > 1e-4 )
          {
          getTransformedPointDifferenceSingleDoubleVtk(inputPoint, bsplineVtk.GetPointer(), true);
          std::cout << "ERROR: Point transform result mismatch between single-precision and double-precision VTK computation at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfSingleDoubleVtkPointMismatches++;
          }
        // Verify VTK derivative
        double derivativeError = getDerivativeErrorVtk(inputPoint, bsplineVtk.GetPointer(), false);
        if ( derivativeError > 1e-6 )
          {
          getDerivativeErrorVtk(inputPoint, bsplineVtk.GetPointer(), true);
          std::cout << "ERROR: Transform derivative result mismatch between VTK and numerical approximation at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfDerivativeMismatches++;
          }
        // Verify VTK inverse transform
        double inverseError = getInverseErrorVtk(inputPoint, bsplineVtk.GetPointer(), false);
        if ( inverseError > 1e-3 )
          {
          getInverseErrorVtk(inputPoint, bsplineVtk.GetPointer(), true);
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
