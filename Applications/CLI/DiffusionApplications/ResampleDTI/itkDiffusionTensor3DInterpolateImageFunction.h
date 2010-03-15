/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __itkDiffusionTensor3DInterpolateImageFunction_h
#define __itkDiffusionTensor3DInterpolateImageFunction_h

#include <itkObject.h>
#include "itkDiffusionTensor3D.h"
#include <itkOrientedImage.h>
#include <itkPoint.h>
//#include <itkSemaphore.h>
#include <itkNumericTraits.h>
#include "define.h"


namespace itk
{
/**
 * \class DiffusionTensor3DInterpolateImageFunction
 * 
 * Virtual class to implement diffusion tensor interpolation classes 
 * 
 */
template< class TData >
class DiffusionTensor3DInterpolateImageFunction : public Object
{
public :
  typedef TData TensorType ;
  typedef DiffusionTensor3DInterpolateImageFunction Self ;
  typedef DiffusionTensor3D< TensorType > TensorDataType ;
  typedef OrientedImage< TensorDataType , 3 > DiffusionImageType ;
  typedef typename DiffusionImageType::Pointer DiffusionImageTypePointer ;
  typedef Point< double , 3 > PointType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  typedef typename TensorDataType::RealValueType TensorRealType ;
  ///Set the input image
  itkSetObjectMacro( InputImage , DiffusionImageType ) ;
  ///Evaluate the tensor value at a given position
  virtual TensorDataType Evaluate( const PointType &point ) = 0 ;
  void SetDefaultPixelValue( TensorRealType defaultPixelValue ) ;
  itkGetMacro( DefaultPixelValue , TensorRealType ) ;

protected:
  DiffusionTensor3DInterpolateImageFunction() ;
  DiffusionImageTypePointer m_InputImage ;
  unsigned long latestTime ;
  TensorRealType m_DefaultPixelValue ;
  TensorDataType m_DefaultPixel ;
};

}//end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DInterpolateImageFunction.txx"
#endif

#endif
