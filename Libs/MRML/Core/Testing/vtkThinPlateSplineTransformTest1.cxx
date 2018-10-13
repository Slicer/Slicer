/*=auto=========================================================================

  Portions (c) Copyright 2010 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"

// ITK includes
#include <itkThinPlateSplineKernelTransform.h>

// VTK includes
#include "vtkImageData.h"
#include "vtkNew.h"
#include "vtkPoints.h"
#include "vtkThinPlateSplineTransform.h"

typedef itk::ThinPlateSplineKernelTransform<double,3> itkThinPlateSplineType;
typedef itkThinPlateSplineType::PointSetType PointSetType;

//----------------------------------------------------------------------------
void CreateThinPlateSplineVtk(vtkThinPlateSplineTransform* tpsTransform, vtkPoints* sourceLandmarks, vtkPoints* targetLandmarks)
{
  tpsTransform->SetBasisToR(); // need to set this because data is 3D
  tpsTransform->SetSourceLandmarks(sourceLandmarks);
  tpsTransform->SetTargetLandmarks(targetLandmarks);
}

//----------------------------------------------------------------------------
itkThinPlateSplineType::Pointer CreateThinPlateSplineItk(vtkPoints* sourceLandmarks, vtkPoints* targetLandmarks)
{
  PointSetType::Pointer sourceLandmarksItk = PointSetType::New();
  for (int i=0; i<sourceLandmarks->GetNumberOfPoints(); i++)
  {
    double pos[3]={0};
    sourceLandmarks->GetPoint(i, pos);
    itkThinPlateSplineType::InputPointType posItk;
    posItk[0]=pos[0];
    posItk[1]=pos[1];
    posItk[2]=pos[2];
    sourceLandmarksItk->GetPoints()->InsertElement(i,posItk);
  }
  PointSetType::Pointer targetLandmarksItk = PointSetType::New();
  for (int i=0; i<targetLandmarks->GetNumberOfPoints(); i++)
  {
    double pos[3]={0};
    targetLandmarks->GetPoint(i, pos);
    itkThinPlateSplineType::InputPointType posItk;
    posItk[0]=pos[0];
    posItk[1]=pos[1];
    posItk[2]=pos[2];
    targetLandmarksItk->GetPoints()->InsertElement(i,posItk);
  }

  itkThinPlateSplineType::Pointer tps = itkThinPlateSplineType::New();
  tps->SetSourceLandmarks(sourceLandmarksItk);
  tps->SetTargetLandmarks(targetLandmarksItk);
  tps->ComputeWMatrix();

  return tps;
}

//----------------------------------------------------------------------------
void AddLandmarkDisplacement(vtkPoints* landmarks, double landmarkIndexX, double landmarkIndexY, double landmarkIndexZ, double dx, double dy, double dz, double dims[3])
{
  double landmarkPosition[3] = {0};
  int landmarkIndex = (landmarkIndexZ*dims[1]+landmarkIndexY)*dims[0]+landmarkIndexX;
  landmarks->GetPoint(landmarkIndex, landmarkPosition);
  landmarkPosition[0] += dx;
  landmarkPosition[1] += dy;
  landmarkPosition[2] += dz;
  landmarks->SetPoint(landmarkIndex, landmarkPosition);
}

//----------------------------------------------------------------------------
// Compute transformed point differences between ITK and VTK ThinPlateSpline implementations
double getTransformedPointDifferenceItkVtk(const double inputPoint[3], itkThinPlateSplineType::Pointer tpsItk, vtkThinPlateSplineTransform* tpsVtk, bool logDetails)
{
  // ITK
  itkThinPlateSplineType::InputPointType inputPointItk;
  inputPointItk[0] = inputPoint[0];
  inputPointItk[1] = inputPoint[1];
  inputPointItk[2] = inputPoint[2];
  itkThinPlateSplineType::OutputPointType outputPointItk;
  outputPointItk = tpsItk->TransformPoint( inputPointItk );

  // VTK
  double outputPoint[3]={0};
  tpsVtk->TransformPoint( inputPoint, outputPoint );

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
// Compute transformed point differences between single-precision and double-precision VTK ThinPlateSpline implementations
double getTransformedPointDifferenceSingleDoubleVtk(const double inputPoint[3], vtkThinPlateSplineTransform* tpsVtk, bool logDetails)
{
  double outputPoint[3]={0};
  tpsVtk->TransformPoint( inputPoint, outputPoint );

  float floatInputPoint[3]={0};
  floatInputPoint[0]=static_cast<float>(inputPoint[0]);
  floatInputPoint[1]=static_cast<float>(inputPoint[1]);
  floatInputPoint[2]=static_cast<float>(inputPoint[2]);
  float floatOutputPoint[3]={0};
  tpsVtk->TransformPoint( floatInputPoint, floatOutputPoint );

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
// Compute the error of derivative computation in VTK ThinPlateSpline implementation
double getDerivativeErrorVtk(const double inputPoint[3], vtkThinPlateSplineTransform* tpsVtk, bool logDetails)
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
    tpsVtk->TransformPoint( xMinus1, xMinus1Transformed);
    double xPlus1Transformed[3]={0};
    tpsVtk->TransformPoint( xPlus1, xPlus1Transformed);
    for (int col=0; col<3; col++)
      {
      jacobianEstimation[col][row] = (xPlus1Transformed[col]-xMinus1Transformed[col])/(2*eps);
      }
    }

  // Jacobian computed by the transform class
  double outputPoint[3]={0};
  double jacobianVtk[3][3];
  tpsVtk->InternalTransformDerivative( inputPoint, outputPoint, jacobianVtk );

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
// Compute the error of inverse computation in VTK ThinPlateSpline implementation
double getInverseErrorVtk(const double inputPoint[3], vtkThinPlateSplineTransform* tpsVtk, bool logDetails)
{
  double outputPoint[3] = { -1, -1, -1 };
  tpsVtk->TransformPoint( inputPoint, outputPoint);

  double inversePoint[3] = { -1, -1, -1 };
  vtkAbstractTransform* inverseTpsVtk=tpsVtk->GetInverse();
  inverseTpsVtk->TransformPoint( outputPoint, inversePoint );

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
int vtkThinPlateSplineTransformTest1(int , char * [] )
{
  // we want to transform a 300x400x300 image, with grid points 100
  // pixels apart. So, we need 4x5x4 "interior" grid points. Since the
  // spline order is 3, we need 1 exterior node on the "left" and 2
  // exterior nodes on the "right".  So, we need a spline grid of
  // 7x8x7. We want to transform the whole image, so the origin should
  // be set to spline grid location (0,0).

  // Create a set of source and target landmark points
  // We pick the points on a regular grid (to make it easier to make some controlled
  // displacements).

  double origin[3] = {-100, -100, -100};
  double spacing[3] = {100, 100, 100};
  double direction[3][3] = {{0.92128500, -0.36017075, -0.146666625}, {0.31722386, 0.91417248, -0.25230478}, {0.22495105, 0.18591857, 0.95646814}};
  double dims[3] = {7,8,7};

  vtkNew<vtkPoints> sourceLandmarks;
  vtkNew<vtkPoints> targetLandmarks;
  for (int z = 0; z < dims[2]; z++)
  {
  for (int y = 0; y < dims[1]; y++)
    {
    for (int x = 0; x < dims[0]; x++)
      {
      double landmarkPosition[3]={0};
      for (int i=0; i<3; i++)
        {
        landmarkPosition[i] = origin[i]+x*spacing[0]*direction[i][0]+y*spacing[1]*direction[i][1]+z*spacing[2]*direction[i][2];
        }
      // Fill with 0 displacements (same position is set for all landmarks in both source and target)
      sourceLandmarks->InsertNextPoint(landmarkPosition);
      targetLandmarks->InsertNextPoint(landmarkPosition);
      }
    }
  }

  // Modify a few landmarks to have non-zero displacements
  AddLandmarkDisplacement(targetLandmarks.GetPointer(), 2, 3, 5, 20, -30, 60, dims);
  AddLandmarkDisplacement(targetLandmarks.GetPointer(), 4, 4, 3, -20, 40, 20, dims);
  AddLandmarkDisplacement(targetLandmarks.GetPointer(), 0, 0, 6, 50, 70, -60, dims);

  // Create an ITK ThinPlateSpline transform. It'll serve as the reference.
  itkThinPlateSplineType::Pointer tpsItk=CreateThinPlateSplineItk(sourceLandmarks.GetPointer(), targetLandmarks.GetPointer());

  // Create a VTK ThinPlateSpline transform with the same parameters.
  vtkNew<vtkThinPlateSplineTransform> tpsVtk;
  CreateThinPlateSplineVtk(tpsVtk.GetPointer(), sourceLandmarks.GetPointer(), targetLandmarks.GetPointer());

  int numberOfPointsTested=0;
  int numberOfItkVtkPointMismatches=0;
  int numberOfSingleDoubleVtkPointMismatches=0;
  int numberOfDerivativeMismatches=0;
  int numberOfInverseMismatches=0;

  // We take samples in the tps region (first node + 2 < node < last node - 1)
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
        double differenceItkVtk = getTransformedPointDifferenceItkVtk(inputPoint, tpsItk, tpsVtk.GetPointer(), false);
        if ( differenceItkVtk > 1e-6 )
          {
          getTransformedPointDifferenceItkVtk(inputPoint, tpsItk, tpsVtk.GetPointer(), true);
          std::cout << "ERROR: Point transform result mismatch between ITK and VTK at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfItkVtkPointMismatches++;
          }
        double differenceSingleDoubleVtk = getTransformedPointDifferenceSingleDoubleVtk(inputPoint, tpsVtk.GetPointer(), false);
        if ( differenceSingleDoubleVtk > 1e-4 )
          {
          getTransformedPointDifferenceSingleDoubleVtk(inputPoint, tpsVtk.GetPointer(), true);
          std::cout << "ERROR: Point transform result mismatch between single-precision and double-precision VTK computation at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfSingleDoubleVtkPointMismatches++;
          }
        // Verify VTK derivative
        double derivativeError = getDerivativeErrorVtk(inputPoint, tpsVtk.GetPointer(), false);
        if ( derivativeError > 1e-6 )
          {
          getDerivativeErrorVtk(inputPoint, tpsVtk.GetPointer(), true);
          std::cout << "ERROR: Transform derivative result mismatch between VTK and numerical approximation at grid point ("<<i<<","<<j<<","<<k<<")"<< std::endl;
          numberOfDerivativeMismatches++;
          }
        // Verify VTK inverse transform
        double inverseError = getInverseErrorVtk(inputPoint, tpsVtk.GetPointer(), false);
        if ( inverseError > 1e-3 )
          {
          getInverseErrorVtk(inputPoint, tpsVtk.GetPointer(), true);
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
