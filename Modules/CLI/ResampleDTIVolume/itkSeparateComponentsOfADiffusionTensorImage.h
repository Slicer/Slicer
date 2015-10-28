/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/CLI/DiffusionApplications/ResampleDTI/itkDiffusionTensor3DResample.h $
  Language:  C++
  Date:      $Date: 2010/04/05 10:04:59 $
  Version:   $Revision: 1.1 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __itkSeparateComponentsOfADiffusionTensorImage_h
#define __itkSeparateComponentsOfADiffusionTensorImage_h

#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkDiffusionTensor3D.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>

namespace itk
{
/** \class itkSeparateComponentsOfADiffusionTensorImage
 *
 *
 */
template <class TInput, class TOutput>
class SeparateComponentsOfADiffusionTensorImage
  : public ImageToImageFilter
  <Image<DiffusionTensor3D<TInput>, 3>,
   Image<TOutput, 3> >
{
public:
  typedef TInput  InputDataType;
  typedef TOutput OutputDataType;
  typedef ImageToImageFilter
  <Image<DiffusionTensor3D<TInput>, 3>,
   Image<TOutput, 3> >
  Superclass;
  typedef DiffusionTensor3D<InputDataType>              InputTensorDataType;
  typedef Image<InputTensorDataType, 3>                 InputImageType;
  typedef SeparateComponentsOfADiffusionTensorImage     Self;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;
  typedef typename InputImageType::Pointer              InputImagePointerType;
  typedef Image<OutputDataType, 3>                      OutputImageType;
  typedef typename OutputImageType::Pointer             OutputImagePointerType;
  typedef itk::ImageRegionIterator<OutputImageType>     OutputIteratorType;
  typedef itk::ImageRegionConstIterator<InputImageType> InputIteratorType;
  typedef typename OutputImageType::RegionType          OutputImageRegionType;
// typedef typename OutputTensorDataType::RealValueType TensorRealType ;

  itkNewMacro( Self );
protected:
  SeparateComponentsOfADiffusionTensorImage();

  void ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType threadId ) ITK_OVERRIDE;

  void GenerateOutputInformation() ITK_OVERRIDE;

  void GenerateInputRequestedRegion() ITK_OVERRIDE;

private:

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSeparateComponentsOfADiffusionTensorImage.txx"
#endif

#endif
