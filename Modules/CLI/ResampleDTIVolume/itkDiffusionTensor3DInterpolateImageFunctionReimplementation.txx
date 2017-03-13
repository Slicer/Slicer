/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DInterpolateImageFunctionReimplementation_txx
#define itkDiffusionTensor3DInterpolateImageFunctionReimplementation_txx

#include "itkDiffusionTensor3DInterpolateImageFunctionReimplementation.h"

namespace itk
{

template <class TData, class TCoordRep>
DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>
::DiffusionTensor3DInterpolateImageFunctionReimplementation()
{
  m_NumberOfThreads = 0;
}

template <class TData, class TCoordRep>
void
DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>
::SetInputImage( const DiffusionImageType *inputImage )
{
  DiffusionTensor3DInterpolateImageFunction<DataType>::SetInputImage( inputImage );   // separateFilter->GetOutput( 0 )
                                                                                      // ) ;
  if( !inputImage )
    {
    return;
    }
  typedef SeparateComponentsOfADiffusionTensorImage<TData, TData> SeparateType;
  typename SeparateType::Pointer separateFilter = SeparateType::New();
  separateFilter->SetInput( inputImage );
  separateFilter->SetNumberOfThreads( this->m_NumberOfThreads );
  separateFilter->Update();
  AllocateInterpolator();
  for( int i = 0; i < 6; i++ )
    {
    m_Interpol[i]->SetInputImage( separateFilter->GetOutput( i ) );
    }

  // this->m_Image = separateFilter->GetOutput( 0 ) ;
}

template <class TData, class TCoordRep>
typename DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>
::TensorDataType
DiffusionTensor3DInterpolateImageFunctionReimplementation<TData, TCoordRep>
::EvaluateAtContinuousIndex( const ContinuousIndexType & index ) const
// ::Evaluate( const PointType &point )
{
  if( this->m_Image.IsNotNull() ) // If input image has been set
    {
    TensorDataType pixelValue;
    for( int i = 0; i < 6; i++ )
      {
      pixelValue[i] = ( DataType ) m_Interpol[i]->EvaluateAtContinuousIndex( index );
      }
    return pixelValue;
    }
  else
    {
    itkExceptionMacro( << "No InputImage Set" );
    }
}

} // end itk namespace

#endif
