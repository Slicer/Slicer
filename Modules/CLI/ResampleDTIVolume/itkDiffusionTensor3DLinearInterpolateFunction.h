/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DLinearInterpolateFunction_h
#define itkDiffusionTensor3DLinearInterpolateFunction_h

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"
#include <itkLinearInterpolateImageFunction.h>

namespace itk
{
/** \class DiffusionTensor3DInterpolateImageFunctionReimplementation
 *
 * Blockwise linear interpolation for diffusion tensor images
 */
template <class TData, class TCoordRep = double>
class DiffusionTensor3DLinearInterpolateFunction :
  public DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DLinearInterpolateFunction<TData, TCoordRep>;
  using Superclass = DiffusionTensor3DInterpolateImageFunctionReimplementation<DataType, TCoordRep>;
  using ImageType = typename Superclass::ImageType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using LinearInterpolateImageFunctionType = LinearInterpolateImageFunction<ImageType, TCoordRep>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DLinearInterpolateFunction, DiffusionTensor3DInterpolateImageFunctionReimplementation);

  itkNewMacro(Self);
protected:
  void AllocateInterpolator() override;

  typename LinearInterpolateImageFunctionType::Pointer linearInterpolator[6];
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DLinearInterpolateFunction.txx"
#endif

#endif
