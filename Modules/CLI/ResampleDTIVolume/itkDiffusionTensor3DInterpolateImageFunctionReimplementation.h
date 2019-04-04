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
  typedef TData                                                          DataType;
  typedef DiffusionTensor3DInterpolateImageFunctionReimplementation      Self;
  typedef DiffusionTensor3DInterpolateImageFunction<DataType, TCoordRep> Superclass;
  typedef typename Superclass::TensorDataType                            TensorDataType;
  typedef typename Superclass::DiffusionImageType                        DiffusionImageType;
  typedef typename Superclass::DiffusionImageTypePointer                 DiffusionImageTypePointer;
  typedef Image<DataType, 3>                                             ImageType;
  typedef typename ImageType::Pointer                                    ImagePointer;
  typedef typename Superclass::PointType                                 PointType;
  typedef SmartPointer<Self>                                             Pointer;
  typedef SmartPointer<const Self>                                       ConstPointer;
  typedef ImageRegionIteratorWithIndex<DiffusionImageType>               IteratorDiffusionImageType;
  typedef ImageRegionIteratorWithIndex<ImageType>                        IteratorImageType;
  typedef InterpolateImageFunction<ImageType, double>                    InterpolateImageFunctionType;
  typedef typename DiffusionImageType::RegionType                        itkRegionType;
  typedef typename DiffusionImageType::SizeType                          SizeType;
  typedef typename Superclass::ContinuousIndexType                       ContinuousIndexType;

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
