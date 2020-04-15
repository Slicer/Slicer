/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DNearestNeighborInterpolateFunction_h
#define itkDiffusionTensor3DNearestNeighborInterpolateFunction_h

#include "itkDiffusionTensor3DInterpolateImageFunction.h"

namespace itk
{

/** \class DiffusionTensor3DInterpolateImageFunction
 *
 * Implementation of the nearest neighborhood interpolation for diffusion tensor images
 */

template <class TData, class TCoordRep = double>
class DiffusionTensor3DNearestNeighborInterpolateFunction :
  public DiffusionTensor3DInterpolateImageFunction<TData, TCoordRep>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DNearestNeighborInterpolateFunction<TData, TCoordRep>;
  using Superclass = DiffusionTensor3DInterpolateImageFunction<DataType>;
  using TensorDataType = typename Superclass::TensorDataType;
  using DiffusionImageType = typename Superclass::DiffusionImageType;
  using PointType = typename Superclass::PointType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using ImageFunctionType = ImageFunction<DiffusionImageType, DataType, double>;
  using ContinuousIndexType = typename Superclass::ContinuousIndexType;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DNearestNeighborInterpolateFunction, DiffusionTensor3DInterpolateImageFunction);

  itkNewMacro( Self );
  // /Evaluate the value of a tensor at a given position
//  TensorDataType Evaluate( const PointType &point ) ;
  TensorDataType EvaluateAtContinuousIndex( const ContinuousIndexType & index ) const override;

protected:
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DNearestNeighborInterpolateFunction.txx"
#endif

#endif
