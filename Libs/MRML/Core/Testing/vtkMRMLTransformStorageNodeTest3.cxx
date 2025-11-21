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

// ITK includes
#include <itkImage.h>
#include <itkVector.h>
#include <itkImageRegionIterator.h>
#include <itkDisplacementFieldTransform.h>
#include <itkTransformFileWriter.h>

// MRML / project testing utilities
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkITKTransformConverter.h"
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
} // namespace

template <typename T>
int TestAffineTransform2DConversionFromITKToVTK(const char* tempDir)
{
  using Affine2DType = itk::AffineTransform<T, 2>;

  // TODO: implement test

  return EXIT_SUCCESS;
}

template <typename T>
int TestVTKOrientedGridTransformFrom2DITKImage(const char* tempDir)
{
  // Typedefs for convenience
  using PixelVectorType = itk::Vector<T, 2>;
  using Image2DType = itk::Image<PixelVectorType, 2>;

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
  it.GoToBegin();
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
  std::string tempFilePath = tempFilename(tempDir, "DispField2D", "tfm", true);
  typename itk::TransformFileWriterTemplate<T>::Pointer writer = itk::TransformFileWriterTemplate<T>::New();
  writer->SetFileName(tempFilePath);
  writer->SetInput(dispTransform);
  TRY_EXPECT_NO_ITK_EXCEPTION(writer->Update());

  // Now read back the displacement field image from the file using Slicer machinery
  vtkNew<vtkMRMLScene> scene;
  scene->SetRootDirectory(tempDir);

  vtkMRMLTransformNode* readTransformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  readTransformNode->AddDefaultStorageNode();
  vtkMRMLTransformStorageNode* readStorageNode = vtkMRMLTransformStorageNode::SafeDownCast(readTransformNode->GetStorageNode());
  readStorageNode->SetFileName(tempFilePath.c_str());
  CHECK_INT(readStorageNode->ReadData(readTransformNode), 1);

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

  std::cout << "TestVTKOrientedGridTransformFrom2DITKImage<" << typeid(T).name() << "> passed." << std::endl;
  return EXIT_SUCCESS;
}

int vtkMRMLTransformStorageNodeTest3(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }

  const char* tempDir = argv[1];

  CHECK_EXIT_SUCCESS(TestAffineTransform2DConversionFromITKToVTK<double>(tempDir));
  CHECK_EXIT_SUCCESS(TestAffineTransform2DConversionFromITKToVTK<float>(tempDir));

  CHECK_EXIT_SUCCESS(TestVTKOrientedGridTransformFrom2DITKImage<double>(tempDir));
  CHECK_EXIT_SUCCESS(TestVTKOrientedGridTransformFrom2DITKImage<float>(tempDir));

  return EXIT_SUCCESS;
}
