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
  typedef TData                                               DataType;
  typedef DiffusionTensor3DNearestNeighborInterpolateFunction Self;
  typedef DiffusionTensor3DInterpolateImageFunction<DataType> Superclass;
  typedef typename Superclass::TensorDataType                 TensorDataType;
  typedef typename Superclass::DiffusionImageType             DiffusionImageType;
  typedef typename Superclass::PointType                      PointType;
  typedef SmartPointer<Self>                                  Pointer;
  typedef SmartPointer<const Self>                            ConstPointer;
  typedef ImageFunction<DiffusionImageType, DataType, double> ImageFunctionType;
  typedef typename Superclass::ContinuousIndexType            ContinuousIndexType;

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
