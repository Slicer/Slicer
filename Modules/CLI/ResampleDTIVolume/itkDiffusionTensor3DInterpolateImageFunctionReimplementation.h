/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DInterpolateImageFunctionReimplementation_h
#define itkDiffusionTensor3DInterpolateImageFunctionReimplementation_h

#include "itkDiffusionTensor3DInterpolateImageFunction.h"
#include <itkImage.h>
#include <itkInterpolateImageFunction.h>
#include "itkSeparateComponentsOfADiffusionTensorImage.h"

namespace itk
{

/**
 * \class DiffusionTensor3DInterpolateImageFunctionReimplementation
 *
 * Abstract class allowing to implement blockwise interpolation for diffusion tensor images
 */

template <class TData, class TCoordRep = double>
class DiffusionTensor3DInterpolateImageFunctionReimplementation :
  public DiffusionTensor3DInterpolateImageFunction<TData, TCoordRep>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>;
  using Superclass = DiffusionTensor3DInterpolateImageFunction<DataType, TCoordRep>;
  using TensorDataType = typename Superclass::TensorDataType;
  using DiffusionImageType = typename Superclass::DiffusionImageType;
  using DiffusionImageTypePointer = typename Superclass::DiffusionImageTypePointer;
  using ImageType = Image<DataType, 3>;
  using ImagePointer = typename ImageType::Pointer;
  using PointType = typename Superclass::PointType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using IteratorDiffusionImageType = ImageRegionIteratorWithIndex<DiffusionImageType>;
  using IteratorImageType = ImageRegionIteratorWithIndex<ImageType>;
  using InterpolateImageFunctionType = InterpolateImageFunction<ImageType, double>;
  using itkRegionType = typename DiffusionImageType::RegionType;
  using SizeType = typename DiffusionImageType::SizeType;
  using ContinuousIndexType = typename Superclass::ContinuousIndexType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DInterpolateImageFunctionReimplementation, DiffusionTensor3DInterpolateImageFunction);

  /** Evaluate the interpolated tensor at a position
   */
  // TensorDataType Evaluate( const PointType &point ) ;
  TensorDataType EvaluateAtContinuousIndex( const ContinuousIndexType & index ) const override;

  void SetInputImage( const DiffusionImageType *inputImage ) override;

  itkSetMacro( NumberOfThreads, int );
protected:
  DiffusionTensor3DInterpolateImageFunctionReimplementation();
  virtual void AllocateInterpolator() = 0;

  typename InterpolateImageFunctionType::Pointer m_Interpol[6];
  ImagePointer m_ImageVec[6];
  int          m_NumberOfThreads;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.txx"
#endif

#endif
