/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

#include <iostream>
#include <string>
#include <cmath>

// VTK includes
#include <vtkNew.h>
#include <vtkImageData.h>
#include <vtkOrientedGridTransform.h>
#include <vtkThinPlateSplineTransform.h>
#include "vtkOrientedBSplineTransform.h"
#include <vtkTransform.h>

// ITK includes
#include <itkImage.h>
#include <itkVector.h>
#include <itkImageRegionIterator.h>
#include <itkDisplacementFieldTransform.h>
#include <itkThinPlateSplineKernelTransform.h>
#include <itkBSplineTransform.h>
#include <itkBSplineDeformableTransform.h>
#include <itkTransformFileWriter.h>
#include <itkAffineTransform.h>
#include <itksys/Directory.hxx>

// MRML / project testing utilities
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLScene.h"

namespace
{
std::string tempFilename(std::string tempDir, std::string suffix, std::string fileExtension, bool remove = false)
{
  std::string filename = tempDir + "/vtkMRMLTransformStorageNodeTest3_" + suffix + "." + fileExtension;
  // remove file if exists
  if (remove && vtksys::SystemTools::FileExists(filename.c_str(), true))
  {
    vtksys::SystemTools::RemoveFile(filename.c_str());
  }
  return filename;
}

template <typename T = vtkMRMLLinearTransformNode>
vtkSmartPointer<T> readTransformUsingSlicer(std::string filePath, vtkMRMLScene* scene)
{
  std::string tempDir = vtksys::SystemTools::GetFilenamePath(filePath);

  vtkNew<T> temp;
  vtkSmartPointer<T> readTransformNode = T::SafeDownCast(scene->AddNewNodeByClass(temp->GetClassName()));
  readTransformNode->AddDefaultStorageNode();
  vtkMRMLTransformStorageNode* readStorageNode = vtkMRMLTransformStorageNode::SafeDownCast(readTransformNode->GetStorageNode());
  readStorageNode->SetFileName(filePath.c_str());
  if (!readStorageNode->ReadData(readTransformNode))
  {
    std::cerr << "Failed to read transform from file: " << filePath << std::endl;
    return nullptr;
  }

  return readTransformNode;
}

int writeTransformUsingSlicer(vtkAbstractTransform* transform, std::string filePath, vtkMRMLScene* scene)
{
  vtkMRMLTransformNode* writeTransformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  writeTransformNode->ApplyTransform(transform);

  writeTransformNode->AddDefaultStorageNode();
  vtkMRMLTransformStorageNode* writeStorageNode = vtkMRMLTransformStorageNode::SafeDownCast(writeTransformNode->GetStorageNode());
  if (!writeStorageNode)
  {
    std::cerr << "Error: Storage node is not created." << std::endl;
    return EXIT_FAILURE;
  }
  writeStorageNode->SetFileName(filePath.c_str());
  CHECK_INT(writeStorageNode->WriteData(writeTransformNode), 1);
  return EXIT_SUCCESS;
}

template <typename T>
int PointsCheck(typename itk::Transform<T, 2, 2>::Pointer itkTransform, vtkMRMLTransformNode* vtkTransformNode, vtkMRMLScene* scene, T tol)
{
  // Now test transforming a set of points
  std::vector<itk::Point<T, 2>> testPointsLps = { itk::Point<T, 2>({ 0.0, 0.0 }),   itk::Point<T, 2>({ 5.0, 10.0 }), itk::Point<T, 2>({ -10.0, 8.0 }),
                                                  itk::Point<T, 2>({ 2.5, 4.5 }),   itk::Point<T, 2>({ 15, -0.5 }),  itk::Point<T, 2>({ 2.2, -8.7 }),
                                                  itk::Point<T, 2>({ 7.8, 14.3 }),  itk::Point<T, 2>({ 25, -9.5 }),  itk::Point<T, 2>({ -3.5, 4.4 }),
                                                  itk::Point<T, 2>({ -1.9, -2.9 }), itk::Point<T, 2>({ -5, -10 }),   itk::Point<T, 2>({ -3.5, 2.25 }) };

  // transform needs to be in the parent node for TransformPointFromWorld to do something
  vtkMRMLTransformNode* dummyTransformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  dummyTransformNode->SetAndObserveTransformNodeID(vtkTransformNode->GetID());

  if constexpr (std::is_same_v<T, float>)
  {
    std::cerr << setprecision(8);
  }
  else
  {
    std::cerr << setprecision(16);
  }

  unsigned failures = 0;
  for (const auto& pointLPS2D : testPointsLps)
  {
    std::cout << "Testing point LPS 2D: [" << pointLPS2D[0] << ", " << pointLPS2D[1] << "]" << std::endl;
    itk::Point<T, 2> itkTransformedPoint2D = itkTransform->TransformPoint(pointLPS2D); // Transform with ITK
    // Convert 2D LPS result to 3D RAS
    itk::Point<T, 3> itkTransformedPoint3D;
    itkTransformedPoint3D[0] = -itkTransformedPoint2D[0];
    itkTransformedPoint3D[1] = -itkTransformedPoint2D[1];
    itkTransformedPoint3D[2] = 0.0;

    // Transform with VTK
    double pointRAS3D[3] = { -pointLPS2D[0], -pointLPS2D[1], 0.0 };
    double slicerTransformedPoint3D[3];
    dummyTransformNode->TransformPointFromWorld(pointRAS3D, slicerTransformedPoint3D);

    // Compare
    for (unsigned int d = 0; d < 3; ++d)
    {
      if (std::abs(slicerTransformedPoint3D[d] - itkTransformedPoint3D[d]) > tol)
      {
        std::cerr << "Transformed point mismatch at dimension " << d << ": expected " << itkTransformedPoint3D[d] << ", got " << slicerTransformedPoint3D[d] << std::endl;
        ++failures;
      }
    }
  }
  if (failures > 0)
  {
    std::cerr << "Total transformed point mismatches: " << failures << ". Tolerance used was: " << tol << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

template <typename T>
typename itk::AffineTransform<T, 2>::Pointer CreateNonTrivialAffine(double thetaDegrees)
{
  using Affine2DType = itk::AffineTransform<T, 2>;
  typename Affine2DType::Pointer affine2d = Affine2DType::New();

  // Center
  typename Affine2DType::InputPointType center{};
  center[0] = 5.0;
  center[1] = 10.0;
  affine2d->SetCenter(center);

  // Define a non-trivial 2x2 matrix (rotation + scaling/shear)
  itk::Matrix<T, 2, 2> mat2d;
  const double theta = vtkMath::RadiansFromDegrees(thetaDegrees);
  mat2d[0][0] = 1.2 * std::cos(theta);
  mat2d[0][1] = -0.8 * std::sin(theta);
  mat2d[1][0] = 0.5 * std::sin(theta);
  mat2d[1][1] = 0.9 * std::cos(theta);
  affine2d->SetMatrix(mat2d);

  // Translation
  typename Affine2DType::OutputVectorType translation;
  translation[0] = 3.5;
  translation[1] = -2.25;
  affine2d->SetTranslation(translation);

  return affine2d;
}

template <typename T>
int TestAffineTransform2DConversionFromITKToVTK(const char* tempDir, vtkMRMLScene* scene)
{
  using Affine2DType = itk::AffineTransform<T, 2>;

  std::cout << "TestAffineTransform2DConversionFromITKToVTK<" << typeid(T).name() << ">" << std::endl;

  // Create an example 2D affine transform
  typename Affine2DType::Pointer affine2d = CreateNonTrivialAffine<T>(15.0);

  // Write it to a temporary file (to test realistic read from file scenario)
  std::string tempFilePath = tempFilename(tempDir, "Affine2D", "tfm", true);
  typename itk::TransformFileWriterTemplate<T>::Pointer writer = itk::TransformFileWriterTemplate<T>::New();
  writer->SetFileName(tempFilePath);
  writer->SetInput(affine2d);
  TRY_EXPECT_NO_ITK_EXCEPTION(writer->Update());

  // Now read back the transform from the file using Slicer machinery
  vtkMRMLLinearTransformNode* readTransformNode = readTransformUsingSlicer<vtkMRMLLinearTransformNode>(tempFilePath, scene);

  vtkTransform* affineTransform = vtkTransform::SafeDownCast(readTransformNode->GetTransformFromParent());
  if (!affineTransform)
  {
    std::cerr << "Converting to vtkTransform failed." << std::endl;
    return EXIT_FAILURE;
  }

  vtkMatrix4x4* vtkMat = affineTransform->GetMatrix();
  if (!vtkMat)
  {
    std::cerr << "vtkTransform has no matrix." << std::endl;
    return EXIT_FAILURE;
  }

  // Write the 3D transform to a new temporary file to ease debugging
  tempFilePath = tempFilename(tempDir, "Affine3D", "tfm", true);
  if (writeTransformUsingSlicer(affineTransform, tempFilePath, scene) != EXIT_SUCCESS)
  {
    std::cerr << "Failed to write transform to file: " << tempFilePath << std::endl;
    return EXIT_FAILURE;
  }

  constexpr double tol = 100 * std::max<double>(std::numeric_limits<T>::epsilon(), std::numeric_limits<itk::SpacePrecisionType>::epsilon());

  return PointsCheck<T>(affine2d, readTransformNode, scene, tol);
}

template <typename T>
int TestThinPlateSpline2DConversionFromITKToVTK(const char* tempDir, vtkMRMLScene* scene)
{
  using ThinPlateSpline2DType = itk::ThinPlateSplineKernelTransform<T, 2>;

  std::cout << "TestThinPlateSpline2DConversionFromITKToVTK<" << typeid(T).name() << ">" << std::endl;

  // Create an example 2D thin plate spline transform
  typename ThinPlateSpline2DType::Pointer tps2d = ThinPlateSpline2DType::New();

  // Add some source and target landmarks for a non-linear transform
  typename ThinPlateSpline2DType::PointSetType::Pointer sourcePoints = ThinPlateSpline2DType::PointSetType::New();
  typename ThinPlateSpline2DType::PointSetType::Pointer targetPoints = ThinPlateSpline2DType::PointSetType::New();
  constexpr unsigned int numberOfLandmarks = 12;
  for (unsigned int n = 0; n < numberOfLandmarks; ++n)
  {
    unsigned i = n / 3;
    unsigned k = n % 3;

    itk::Point<T, 2> sourcePoint;
    sourcePoint[0] = i * 12.5 - 25.5;
    sourcePoint[1] = k * 7.5 - 20.0;
    sourcePoints->SetPoint(n, sourcePoint);

    itk::Point<T, 2> targetPoint;
    targetPoint[0] = sourcePoint[0] + std::sin(n) * 0.2;
    targetPoint[1] = sourcePoint[1] + std::cos(n) * 1.3;
    targetPoints->SetPoint(n, targetPoint);
  }
  tps2d->SetSourceLandmarks(sourcePoints);
  tps2d->SetTargetLandmarks(targetPoints);
  tps2d->ComputeWMatrix();

  // Write it to a temporary file (to test realistic read from file scenario)
  std::string tempFilePath = tempFilename(tempDir, "ThinPlateSpline2D", "tfm", true);
  typename itk::TransformFileWriterTemplate<T>::Pointer writer = itk::TransformFileWriterTemplate<T>::New();
  writer->SetFileName(tempFilePath);
  writer->SetInput(tps2d);
  TRY_EXPECT_NO_ITK_EXCEPTION(writer->Update());

  // Now read back the transform from the file using Slicer machinery
  vtkMRMLTransformNode* readTransformNode = readTransformUsingSlicer<vtkMRMLTransformNode>(tempFilePath, scene);

  vtkThinPlateSplineTransform* tpsT = vtkThinPlateSplineTransform::SafeDownCast(readTransformNode->GetTransformFromParent());
  if (!tpsT)
  {
    std::cerr << "Converting to vtkThinPlateSplineTransform failed." << std::endl;
    return EXIT_FAILURE;
  }

  // Write the 3D transform to a new temporary file to ease debugging
  tempFilePath = tempFilename(tempDir, "ThinPlateSpline3D", "tfm", true);
  if (writeTransformUsingSlicer(tpsT, tempFilePath, scene) != EXIT_SUCCESS)
  {
    std::cerr << "Failed to write transform to file: " << tempFilePath << std::endl;
    return EXIT_FAILURE;
  }

  constexpr double tol = 1000 * std::max<double>(std::numeric_limits<T>::epsilon(), std::numeric_limits<itk::SpacePrecisionType>::epsilon());

  return PointsCheck<T>(tps2d, readTransformNode, scene, tol);
}

template <typename T>
int TestVTKOrientedGridTransformFrom2DITKImage(const char* tempDir, vtkMRMLScene* scene)
{
  // Typedefs for convenience
  using PixelVectorType = itk::Vector<T, 2>;
  using Image2DType = itk::Image<PixelVectorType, 2>;

  std::cout << "TestVTKOrientedGridTransformFrom2DITKImage<" << typeid(T).name() << ">" << std::endl;

  // Create a small 2D displacement field
  typename Image2DType::Pointer dispImage = Image2DType::New();

  typename Image2DType::SizeType size;
  size[0] = 161; // X
  size[1] = 173; // Y
  typename Image2DType::RegionType region;
  region.SetSize(size);
  dispImage->SetRegions(region);

  // Set origin, spacing and a non-trivial direction (to exercise conversion)
  typename Image2DType::PointType origin;
  origin[0] = -32.4;
  origin[1] = -23.5;
  dispImage->SetOrigin(origin);

  typename Image2DType::SpacingType spacing;
  spacing[0] = 1.85;
  spacing[1] = 0.73;
  dispImage->SetSpacing(spacing);

  typename Image2DType::DirectionType direction;
  direction.SetIdentity();
  // apply a simple 2D rotation for direction to ensure conversion handles non-identity
  const double theta = vtkMath::RadiansFromDegrees(5.0);
  direction(0, 0) = std::cos(theta);
  direction(0, 1) = -std::sin(theta);
  direction(1, 0) = std::sin(theta);
  direction(1, 1) = std::cos(theta);
  dispImage->SetDirection(direction);

  dispImage->Allocate();

  // Fill image with deterministic values: pixel(x,y) = [x + 0.1*y, 2*x - 0.5*y]
  itk::ImageRegionIteratorWithIndex<Image2DType> it(dispImage, dispImage->GetRequestedRegion());
  while (!it.IsAtEnd())
  {
    typename Image2DType::IndexType idx = it.GetIndex();
    PixelVectorType v;
    v[0] = idx[0] + 0.1 * idx[1];
    v[1] = 2.0 * idx[0] - 0.5 * idx[1];
    it.Set(v);
    ++it;
  }

  // Write it to a temporary file (to test realistic read from file scenario)
  typename itk::DisplacementFieldTransform<T, 2>::Pointer dispTransform = itk::DisplacementFieldTransform<T, 2>::New();
  dispTransform->SetDisplacementField(dispImage);
  std::string tempFilePath = tempFilename(tempDir, "DisplacementField2D", "tfm", true);
  typename itk::TransformFileWriterTemplate<T>::Pointer writer = itk::TransformFileWriterTemplate<T>::New();
  writer->SetFileName(tempFilePath);
  writer->SetInput(dispTransform);
  TRY_EXPECT_NO_ITK_EXCEPTION(writer->Update());

  // Now read back the displacement field image from the file using Slicer machinery
  vtkMRMLTransformNode* readTransformNode = readTransformUsingSlicer<vtkMRMLTransformNode>(tempFilePath, scene);

  vtkOrientedGridTransform* gridTransform = vtkOrientedGridTransform::SafeDownCast(readTransformNode->GetTransformFromParent());
  if (!gridTransform)
  {
    std::cerr << "Converting to vtkOrientedGridTransform failed." << std::endl;
    return EXIT_FAILURE;
  }

  vtkImageData* gridImage_Ras = gridTransform->GetDisplacementGrid();
  if (!gridImage_Ras)
  {
    std::cerr << "Resulting vtkImageData is null." << std::endl;
    return EXIT_FAILURE;
  }

  // Now compare it to the original displacement field
  // Dimensions should match X,Y and Z==1
  int dims[3];
  gridImage_Ras->GetDimensions(dims);
  if (dims[0] != static_cast<int>(size[0]) || dims[1] != static_cast<int>(size[1]) || dims[2] != 1)
  {
    std::cerr << "Dimensions mismatch: expected (" << size[0] << "," << size[1] << ",1) got (" << dims[0] << "," << dims[1] << "," << dims[2] << ")" << std::endl;
    return EXIT_FAILURE;
  }

  // Number of scalar components must be 3 (expanded 2D->3D vector)
  if (gridImage_Ras->GetNumberOfScalarComponents() != 3)
  {
    std::cerr << "Scalar components mismatch: expected 3 got " << gridImage_Ras->GetNumberOfScalarComponents() << std::endl;
    return EXIT_FAILURE;
  }

  // Origin: conversion flips X and Y signs (LPS->RAS) and Z is 0
  double origin_ras[3];
  gridImage_Ras->GetOrigin(origin_ras);
  double expected_origin_ras[3] = { -origin[0], -origin[1], 0.0 };
  constexpr double tol = 100 * std::max<double>(std::numeric_limits<T>::epsilon(), std::numeric_limits<itk::SpacePrecisionType>::epsilon());
  for (int i = 0; i < 3; ++i)
  {
    if (std::abs(origin_ras[i] - expected_origin_ras[i]) > tol)
    {
      std::cerr << "Origin mismatch at index " << i << ": expected " << expected_origin_ras[i] << " got " << origin_ras[i] << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Spacing: X,Y from ITK, ignore Z
  double spacing_ras[3];
  gridImage_Ras->GetSpacing(spacing_ras);
  if (std::abs(spacing_ras[0] - spacing[0]) > tol || std::abs(spacing_ras[1] - spacing[1]) > tol)
  {
    std::cerr << "Spacing XY mismatch: expected (" << spacing[0] << "," << spacing[1] << ") got (" << spacing_ras[0] << "," << spacing_ras[1] << ")" << std::endl;
    return EXIT_FAILURE;
  }

  // Validate vector values: they should match conversion performed in SetVTKOrientedGridTransformFrom2DITKImage:
  // vtkX = -itkX, vtkY = -itkY, vtkZ = 0.0
  itk::SpacePrecisionType* ptr = reinterpret_cast<itk::SpacePrecisionType*>(gridImage_Ras->GetScalarPointer());
  itk::ImageRegionConstIterator<Image2DType> it2(dispImage, dispImage->GetRequestedRegion());
  while (!it2.IsAtEnd())
  {
    PixelVectorType itkV = it2.Get();
    T expected0 = -itkV[0];
    T expected1 = -itkV[1];
    T expected2 = 0.0;

    itk::SpacePrecisionType actual0 = *(ptr++);
    itk::SpacePrecisionType actual1 = *(ptr++);
    itk::SpacePrecisionType actual2 = *(ptr++);

    if (std::abs(actual0 - expected0) > tol || std::abs(actual1 - expected1) > tol || std::abs(actual2 - expected2) > tol)
    {
      std::cerr << "Vector mismatch at index (" << it2.GetIndex()[0] << "," << it2.GetIndex()[1] << "): ";
      std::cerr << "expected (" << expected0 << "," << expected1 << "," << expected2 << ") got (";
      std::cerr << actual0 << "," << actual1 << "," << actual2 << "). Tolerance: " << tol << std::endl;
      return EXIT_FAILURE;
    }
    ++it2;
  }

  // Write the 3D transform to a new temporary file to ease debugging
  tempFilePath = tempFilename(tempDir, "DisplacementField3D", "tfm", true);
  if (writeTransformUsingSlicer(gridTransform, tempFilePath, scene) != EXIT_SUCCESS)
  {
    std::cerr << "Failed to write transform to file: " << tempFilePath << std::endl;
    return EXIT_FAILURE;
  }

  return PointsCheck<T>(dispTransform, readTransformNode, scene, 1e-4);
}

template <typename TransformType>
typename TransformType::Pointer CreateNonTrivialBSpline()
{
  // Typedefs for convenience
  using Image2DType = typename TransformType::ImageType;

  // Create a small 2D displacement field
  typename TransformType::ImagePointer dispImageX = TransformType::ImageType::New();

  typename Image2DType::SizeType size;
  size[0] = 16; // X
  size[1] = 17; // Y
  typename Image2DType::RegionType region;
  region.SetSize(size);
  dispImageX->SetRegions(region);

  // Set origin, spacing and a non-trivial direction (to exercise conversion)
  typename Image2DType::PointType origin;
  origin[0] = -32.4;
  origin[1] = -23.5;
  dispImageX->SetOrigin(origin);

  typename Image2DType::SpacingType spacing;
  spacing[0] = 18.5;
  spacing[1] = 7.3;
  dispImageX->SetSpacing(spacing);

  typename Image2DType::DirectionType direction;
  direction.SetIdentity();
  // apply a simple 2D rotation for direction to ensure conversion handles non-identity
  const double theta = vtkMath::RadiansFromDegrees(5.0);
  direction(0, 0) = std::cos(theta);
  direction(0, 1) = -std::sin(theta);
  direction(1, 0) = std::sin(theta);
  direction(1, 1) = std::cos(theta);
  dispImageX->SetDirection(direction);

  dispImageX->Allocate();

  typename Image2DType::Pointer dispImageY = Image2DType::New();
  dispImageY->SetRegions(region);
  dispImageY->CopyInformation(dispImageX);
  dispImageY->Allocate();

  // Fill image with deterministic values, make it similar to TestVTKOrientedGridTransformFrom2DITKImage
  itk::ImageRegionIteratorWithIndex<Image2DType> itX(dispImageX, dispImageX->GetRequestedRegion());
  itk::ImageRegionIteratorWithIndex<Image2DType> itY(dispImageY, dispImageY->GetRequestedRegion());
  while (!itX.IsAtEnd())
  {
    typename Image2DType::IndexType idx = itX.GetIndex();
    itX.Set(10 * (idx[0] + 0.1 * idx[1]));
    itY.Set(10 * (2.0 * idx[0] - 0.5 * idx[1]));
    ++itX;
    ++itY;
  }

  auto bsplineTransform = TransformType::New();
  typename TransformType::ImagePointer images[2];
  images[0] = dispImageX;
  images[1] = dispImageY;
  bsplineTransform->SetCoefficientImages(images);

  return bsplineTransform;
}

template <typename T>
int TestVTKBSplineParametersFromITK(const char* tempDir, vtkMRMLScene* scene)
{
  using TransformType = itk::BSplineTransform<T, 2, 3>;
  std::cout << "TestVTKBSplineParametersFromITK<" << typeid(T).name() << ">" << std::endl;

  typename TransformType::Pointer bsplineTransform = CreateNonTrivialBSpline<TransformType>();

  // Write it to a temporary file (to test realistic read from file scenario)
  std::string tempFilePath = tempFilename(tempDir, "BSpline2D", "tfm", true);
  typename itk::TransformFileWriterTemplate<T>::Pointer writer = itk::TransformFileWriterTemplate<T>::New();
  writer->SetFileName(tempFilePath);
  writer->SetInput(bsplineTransform);
  TRY_EXPECT_NO_ITK_EXCEPTION(writer->Update());

  // Now read back the displacement field image from the file using Slicer machinery
  vtkMRMLTransformNode* readTransformNode = readTransformUsingSlicer<vtkMRMLTransformNode>(tempFilePath, scene);

  vtkOrientedBSplineTransform* vtkTransform = vtkOrientedBSplineTransform::SafeDownCast(readTransformNode->GetTransformFromParent());
  if (!vtkTransform)
  {
    std::cerr << "Converting to vtkOrientedBSplineTransform failed." << std::endl;
    return EXIT_FAILURE;
  }

  // Write the 3D transform to a new temporary file to ease debugging
  tempFilePath = tempFilename(tempDir, "BSpline3D", "tfm", true);
  // Writing needs inversion, I don't know how to force it here
  // if (writeTransformUsingSlicer(vtkTransform, tempFilePath, scene) != EXIT_SUCCESS)
  //{
  //  std::cerr << "Failed to write transform to file: " << tempFilePath << std::endl;
  //  return EXIT_FAILURE;
  //}

  constexpr double tol = 1000 * std::max<double>(std::numeric_limits<T>::epsilon(), std::numeric_limits<itk::SpacePrecisionType>::epsilon());

  return PointsCheck<T>(bsplineTransform, readTransformNode, scene, tol);
}

template <typename T>
int TestVTKBSplineParametersFromITKv3(const char* tempDir, vtkMRMLScene* scene)
{
  // Typedefs for convenience
  using TransformType = itk::BSplineDeformableTransform<T, 2, 3>;
  std::cout << "TestVTKBSplineParametersFromITKv3<" << typeid(T).name() << ">" << std::endl;

  typename TransformType::Pointer bsplineTransform = CreateNonTrivialBSpline<TransformType>();
  // bsplineTransform->SetBulkTransform(CreateNonTrivialAffine<T>(0.1));  // does not get written to file

  // Write it to a temporary file (to test realistic read from file scenario)
  std::string tempFilePath = tempFilename(tempDir, "BSpline2Dv3", "tfm", true);
  typename itk::TransformFileWriterTemplate<T>::Pointer writer = itk::TransformFileWriterTemplate<T>::New();
  writer->SetFileName(tempFilePath);
  writer->SetInput(bsplineTransform);
  TRY_EXPECT_NO_ITK_EXCEPTION(writer->Update());

  // Now read back the displacement field image from the file using Slicer machinery
  vtkMRMLTransformNode* readTransformNode = readTransformUsingSlicer<vtkMRMLTransformNode>(tempFilePath, scene);

  vtkOrientedBSplineTransform* vtkTransform = vtkOrientedBSplineTransform::SafeDownCast(readTransformNode->GetTransformFromParent());
  if (!vtkTransform)
  {
    std::cerr << "Converting to vtkOrientedBSplineTransform failed." << std::endl;
    return EXIT_FAILURE;
  }

  // Write the 3D transform to a new temporary file to ease debugging
  tempFilePath = tempFilename(tempDir, "BSpline3Dv3", "tfm", true);
  // Writing needs inversion, I don't know how to force it here
  // if (writeTransformUsingSlicer(vtkTransform, tempFilePath, scene) != EXIT_SUCCESS)
  //{
  //  std::cerr << "Failed to write transform to file: " << tempFilePath << std::endl;
  //  return EXIT_FAILURE;
  //}

  constexpr double tol = 1000 * std::max<double>(std::numeric_limits<T>::epsilon(), std::numeric_limits<itk::SpacePrecisionType>::epsilon());

  return PointsCheck<T>(bsplineTransform, readTransformNode, scene, tol);
}
} // namespace

int vtkMRMLTransformStorageNodeTest3(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  const char* tempDir = argv[1];

  vtkNew<vtkMRMLScene> scene;
  scene->SetRootDirectory(tempDir);

  CHECK_EXIT_SUCCESS(TestAffineTransform2DConversionFromITKToVTK<double>(tempDir, scene));
  CHECK_EXIT_SUCCESS(TestAffineTransform2DConversionFromITKToVTK<float>(tempDir, scene));

  CHECK_EXIT_SUCCESS(TestThinPlateSpline2DConversionFromITKToVTK<double>(tempDir, scene));
  CHECK_EXIT_SUCCESS(TestThinPlateSpline2DConversionFromITKToVTK<float>(tempDir, scene));

  CHECK_EXIT_SUCCESS(TestVTKOrientedGridTransformFrom2DITKImage<double>(tempDir, scene));
  CHECK_EXIT_SUCCESS(TestVTKOrientedGridTransformFrom2DITKImage<float>(tempDir, scene));

  CHECK_EXIT_SUCCESS(TestVTKBSplineParametersFromITK<double>(tempDir, scene));
  CHECK_EXIT_SUCCESS(TestVTKBSplineParametersFromITK<float>(tempDir, scene));

  CHECK_EXIT_SUCCESS(TestVTKBSplineParametersFromITKv3<double>(tempDir, scene));
  CHECK_EXIT_SUCCESS(TestVTKBSplineParametersFromITKv3<float>(tempDir, scene));

  return EXIT_SUCCESS;
}
