/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef __itkDiffusionTensor3DInterpolateImageFunctionReimplementation_h
#define __itkDiffusionTensor3DInterpolateImageFunctionReimplementation_h

#include "itkDiffusionTensor3DInterpolateImageFunction.h"
#include <itkOrientedImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkInterpolateImageFunction.h>
#include <itkMutexLock.h>
#include <itkSemaphore.h>

namespace itk
{

struct RegionType
{
  ImageRegion< 3 > itkRegion ;
  bool Done ;
  Index< 3 > PositionInImage ;
  bool Stop ;
};

/**
 * \class DiffusionTensor3DInterpolateImageFunctionReimplementation
 * 
 * Abstract class allowing to implement blockwise interpolation for diffusion tensor images
 */

template< class TData >
class DiffusionTensor3DInterpolateImageFunctionReimplementation :
  public DiffusionTensor3DInterpolateImageFunction< TData >
{
public :
  typedef TData DataType ;
  typedef DiffusionTensor3DInterpolateImageFunctionReimplementation Self ;
  typedef DiffusionTensor3DInterpolateImageFunction< DataType > Superclass ;
  typedef typename Superclass::TensorDataType TensorDataType ;
  typedef typename Superclass::DiffusionImageType DiffusionImageType ;
  typedef typename Superclass::DiffusionImageTypePointer DiffusionImageTypePointer ;
  typedef OrientedImage< DataType , 3 > ImageType ;
  typedef typename ImageType::Pointer ImagePointer ;
  typedef typename Superclass::PointType PointType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  typedef ImageRegionIteratorWithIndex< DiffusionImageType > IteratorDiffusionImageType ;
  typedef ImageRegionIteratorWithIndex< ImageType > IteratorImageType ;
  typedef InterpolateImageFunction< ImageType , double > InterpolateImageFunctionType ;
  typedef typename DiffusionImageType::RegionType itkRegionType ;
  typedef typename DiffusionImageType::SizeType SizeType ;
  /** Evaluate the interpolated tensor at a position
   */
  TensorDataType Evaluate( const PointType &point ) ;
  virtual void SetInputImage( DiffusionImageTypePointer inputImage ) ;
protected:
  DiffusionTensor3DInterpolateImageFunctionReimplementation() ;
  virtual void AllocateInterpolator() = 0 ;
  void SeparateImages() ;
  void AllocateImages() ;
  bool DivideRegion( int currentThread ) ;
  int RegionToDivide() ;
  typename InterpolateImageFunctionType::Pointer m_Interpol[ 6 ] ;
  ImagePointer m_Image[ 6 ] ;
  Semaphore::Pointer m_Threads ;
  int m_SplitAxis ;
  bool m_SeparationDone ;
  bool m_CannotSplit ;
  MutexLock::Pointer m_Lock ;
  MutexLock::Pointer m_LockNewThreadDetected ;
  std::vector< RegionType > m_ListRegions ;
  int m_NbThread ;
  MutexLock::Pointer m_CheckRegionsDone ;
  bool m_ExceptionThrown ;
  SizeType m_Size ;
  bool m_AllocateInterpolatorsDone ;
};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.txx"
#endif

#endif
