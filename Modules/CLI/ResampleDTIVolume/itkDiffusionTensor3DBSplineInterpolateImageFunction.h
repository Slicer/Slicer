/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DBSplineInterpolateImageFunction_h
#define itkDiffusionTensor3DBSplineInterpolateImageFunction_h

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"
#include <itkBSplineInterpolateImageFunction.h>
#include <itkImage.h>

namespace itk
{
/**
 * \class DiffusionTensor3DSplineInterpolateImageFunction
 *
 * Implementation of blockwise spline interpolation for diffusion tensor images
 */
template <class TData, class TCoordRep = double>
class DiffusionTensor3DBSplineInterpolateImageFunction :
  public DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DBSplineInterpolateImageFunction<TData, TCoordRep>;
  using Superclass = DiffusionTensor3DInterpolateImageFunctionReimplementation<DataType, TCoordRep>;
  using ImageType = typename Superclass::ImageType;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using BSplineInterpolateFunction = BSplineInterpolateImageFunction<ImageType, TCoordRep, double>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DBSplineInterpolateImageFunction, DiffusionTensor3DInterpolateImageFunctionReimplementation);

  itkNewMacro( Self );
  // /Get the Spline Order, supports 0th - 5th order splines. The default is a 1st order spline.
  itkGetMacro( SplineOrder, int );
  // /Set the Spline Order, supports 0th - 5th order splines. The default is a 1st order spline.
  itkSetMacro( SplineOrder, unsigned int );
protected:
  void AllocateInterpolator() override;

  DiffusionTensor3DBSplineInterpolateImageFunction();
  unsigned int m_SplineOrder;
  typename BSplineInterpolateFunction::Pointer bSplineInterpolateFunction[6];
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DBSplineInterpolateImageFunction.txx"
#endif

#endif
