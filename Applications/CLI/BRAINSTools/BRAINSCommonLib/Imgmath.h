/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile$
 *  Language:  C++
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#ifndef __Imgmath_h
#define __Imgmath_h

/* This file contains the functions to perform pixel by pixel mathematical
  * operations on 2 images. All the functions are performed by using ITK
  * filters. */

#include "itkImage.h"
#include "itkSquaredDifferenceImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkSquaredDifferenceImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkDivideImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkMinimumImageFilter.h"
#include "itkMaximumImageFilter.h"
// TODO:  add these correctly so we get multi-threading.
#include "itkInvertIntensityImageFilter.h"
#include "itkAddConstantToImageFilter.h"
#include "itkDivideByConstantImageFilter.h"
#include "itkMultiplyByConstantImageFilter.h"
#include "itkSqrtImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkExpNegativeImageFilter.h"
#include "itkLogImageFilter.h"
#include <vcl_cmath.h>

/* Iadd adds 2 images at every pixel location and outputs the resulting image.*/
template< class ImageType >
typename ImageType::Pointer Iadd(const typename ImageType::Pointer input1,
                                 typename ImageType::Pointer input2)
{
  typedef itk::AddImageFilter< ImageType, ImageType, ImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1(input1);
  filter->SetInput2(input2);
  try
    {
    filter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit(-1);
    }
  typename ImageType::Pointer image = filter->GetOutput();
  return image;
}

/* Isub subtracts 2 images at every pixel location and outputs the resulting
  * image.*/
template< class ImageType >
typename ImageType::Pointer Isub(const typename ImageType::Pointer input1,
                                 const typename ImageType::Pointer input2)
{
  typedef itk::SubtractImageFilter< ImageType, ImageType, ImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1(input1);
  filter->SetInput2(input2);
  try
    {
    filter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit(-1);
    }
  typename ImageType::Pointer image = filter->GetOutput();
  return image;
}

/* Imul multiplies 2 images at every pixel location and outputs the resulting
  * image.*/
template< class ImageType >
typename ImageType::Pointer Imul(const typename ImageType::Pointer input1,
                                 const typename ImageType::Pointer input2)
{
  typedef itk::MultiplyImageFilter< ImageType, ImageType, ImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1(input1);
  filter->SetInput2(input2);
  try
    {
    filter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit(-1);
    }
  typename ImageType::Pointer image = filter->GetOutput();
  return image;
}

/* Idiv divides 2 images at every pixel location and outputs the resulting
  * image.*/
template< class ImageType >
typename ImageType::Pointer Idiv(const typename ImageType::Pointer input1,
                                 const typename ImageType::Pointer input2)
{
  typedef itk::DivideImageFilter< ImageType, ImageType, ImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1(input1);
  filter->SetInput2(input2);
  try
    {
    filter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit(-1);
    }
  typename ImageType::Pointer image = filter->GetOutput();
  return image;
}

/* Imax does the numerical generalization of OR on 2 (non-negative) images at
  * every pixel location
  * and outputs the resulting image.*/
template< class ImageType >
typename ImageType::Pointer Imax(const typename ImageType::Pointer input1,
                                 const typename ImageType::Pointer input2)
{
  typedef itk::MaximumImageFilter< ImageType, ImageType, ImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1(input1);
  filter->SetInput2(input2);
  try
    {
    filter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit(-1);
    }
  typename ImageType::Pointer image = filter->GetOutput();
  return image;
}

/* Imin does the numerical generalization of AND on 2 (non-negative) images at
  * every pixel location
  * and outputs the resulting image.*/
template< class ImageType >
typename ImageType::Pointer Imin(const typename ImageType::Pointer input1,
                                 const typename ImageType::Pointer input2)
{
  typedef itk::MinimumImageFilter< ImageType, ImageType, ImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput1(input1);
  filter->SetInput2(input2);
  try
    {
    filter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit(-1);
    }
  typename ImageType::Pointer image = filter->GetOutput();
  return image;
}

template< class ImageType >
typename ImageType::Pointer IMask(typename ImageType::Pointer input1,
                                  typename ImageType::Pointer mask)
{
  typename ImageType::Pointer image = ImageType::New();
  image->SetRegions( input1->GetLargestPossibleRegion() );
  image->CopyInformation(input1);
  image->Allocate();
  typedef typename itk::ImageRegionIterator< ImageType > RegionIteratorType;
  RegionIteratorType in1( input1, input1->GetLargestPossibleRegion() );
  RegionIteratorType in2( mask, mask->GetLargestPossibleRegion() );
  RegionIteratorType out( image, image->GetLargestPossibleRegion() );
  for ( in1.GoToBegin(), out.GoToBegin(), in2.GoToBegin();
        !in1.IsAtEnd();
        ++in1, ++in2, ++out )
    {
    const typename ImageType::PixelType temp = in1.Get();
    out.Set( ( in2.Get() > 0 ) ? temp : 0 );
    }
  return image;
}

template< class ImageType >
typename ImageType::Pointer ImageAddConstant(
  const typename ImageType::Pointer input,
  const double shiftvalue)
{
  // TODO:  This should be a UnaryImageFunctor operation to get multi-threading.
  typename ImageType::Pointer outImage = ImageType::New();
  outImage->SetRegions( input->GetLargestPossibleRegion() );
  outImage->CopyInformation(input);
  outImage->Allocate();
  typedef typename itk::ImageRegionIterator< ImageType > RegionIteratorType;
  RegionIteratorType in( input, input->GetLargestPossibleRegion() );
  RegionIteratorType out( outImage, outImage->GetLargestPossibleRegion() );
  out.GoToBegin();
  for ( in.GoToBegin(); !in.IsAtEnd(); ++in )
    {
    out.Set( static_cast< typename ImageType::PixelType >( ( in.Get()
                                                             + shiftvalue ) ) );
    ++out;
    }
  return outImage;
}

template< class ImageType >
typename ImageType::Pointer ImageMultiplyConstant(
  const typename ImageType::Pointer input,
  const double scalevalue)
{
  typedef typename itk::MultiplyByConstantImageFilter< ImageType, double, ImageType >
  MultByConstFilterType;
  typedef typename MultByConstFilterType::Pointer MultByConstFilterPointer;
  MultByConstFilterPointer filt = MultByConstFilterType::New();
  filt->SetInput(input);
  filt->SetConstant(scalevalue);
  filt->Update();
  return filt->GetOutput();
}

/* ImageComplementConstant does the numerical generalization of NOT on one
  * (non-negative) image
  * at every pixel location and outputs the resulting image.  For finding the
  *binary mask image Not,
  * referencevalue should be 1;  however there is no defaulting to 1 here. */
template< class ImageType >
typename ImageType::Pointer ImageComplementConstant(
  const typename ImageType::Pointer input,
  const double referencevalue)
{
  // TODO:  This should be a UnaryImageFunctor operation to get multi-threading.
  typename ImageType::Pointer outImage = ImageType::New();
  outImage->SetRegions( input->GetLargestPossibleRegion() );
  outImage->CopyInformation(input);
  outImage->Allocate();
  typedef typename itk::ImageRegionIterator< ImageType > RegionIteratorType;
  RegionIteratorType in( input, input->GetLargestPossibleRegion() );
  RegionIteratorType out( outImage, outImage->GetLargestPossibleRegion() );
  out.GoToBegin();
  for ( in.GoToBegin(); !in.IsAtEnd(); ++in )
    {
    out.Set( static_cast< typename ImageType::PixelType >( ( referencevalue
                                                             - in.Get() ) ) );
    ++out;
    }
  return outImage;
}

template< class ImageType >
typename ImageType::Pointer ImageDivideConstant(
  typename ImageType::Pointer input,
  const double denominator)
{
  typename ImageType::Pointer DivImage = ImageMultiplyConstant(input,
                                                               1.0 / denominator);
  return DivImage;
}

template< class ImageType >
void ImageSqrtValue(typename ImageType::Pointer Output,
                    const typename ImageType::Pointer Input)
{
  typename ImageType::Pointer image = ImageType::New();
  image->SetRegions( Input->GetLargestPossibleRegion() );
  image->CopyInformation(Input);
  image->Allocate();
  typedef typename itk::ImageRegionIterator< ImageType > RegionIteratorType;
  RegionIteratorType in( Input, Input->GetLargestPossibleRegion() );
  RegionIteratorType out( image, image->GetLargestPossibleRegion() );
  for ( in.GoToBegin(), out.GoToBegin(); !in.IsAtEnd(); ++in, ++out )
    {
    out.Set( static_cast< typename ImageType::PixelType >( in.Get() ) );
    }

  Output = image;
  return;
}

template< class ImageType >
typename ImageType::Pointer
ImageSqrtValue(const typename ImageType::Pointer input)
{
  typename ImageType::Pointer rval;
  ImageSqrtValue< ImageType >(rval, input);
  return rval;
}

#endif
