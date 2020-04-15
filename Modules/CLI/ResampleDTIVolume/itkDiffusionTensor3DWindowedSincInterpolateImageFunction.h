/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DWindowedSincInterpolateImageFunction_h
#define itkDiffusionTensor3DWindowedSincInterpolateImageFunction_h

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"
#include <itkWindowedSincInterpolateImageFunction.h>
#include <itkConstantBoundaryCondition.h>
#include <itkImage.h>

namespace itk
{
/**
 * \class DiffusionTensor3DWindowedSincInterpolateImageFunction
 *
 * Implementation of windowed sinc blockwise interpolation for diffusion tensor images
 */
template <class TData,
          unsigned int VRadius,
          class TWindowFunction = Function::HammingWindowFunction<VRadius>,
          class TBoundaryCondition = ConstantBoundaryCondition<Image<TData, 3> >,
          class TCoordRep = double
          >
class DiffusionTensor3DWindowedSincInterpolateImageFunction
  : public DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DWindowedSincInterpolateImageFunction<TData, VRadius, TWindowFunction, TBoundaryCondition, TCoordRep>;
  using Superclass = DiffusionTensor3DInterpolateImageFunctionReimplementation<DataType, TCoordRep>;
  using ImageType = typename Superclass::ImageType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using WindowedSincInterpolateImageFunctionType = WindowedSincInterpolateImageFunction<ImageType, VRadius, TWindowFunction, TBoundaryCondition, TCoordRep>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DWindowedSincInterpolateImageFunction, DiffusionTensor3DInterpolateImageFunctionReimplementation);

  itkNewMacro(Self);
protected:
  void AllocateInterpolator() override;

  typename WindowedSincInterpolateImageFunctionType::Pointer windowedSincInterpolator[6];
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DWindowedSincInterpolateImageFunction.txx"
#endif

#endif
