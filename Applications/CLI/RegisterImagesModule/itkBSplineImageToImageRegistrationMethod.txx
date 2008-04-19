/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: MomentRegistrator.txx,v $
  Language:  C++
  Date:      $Date: 2007/03/29 17:52:55 $
  Version:   $Revision: 1.6 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef __BSplineImageToImageRegistrationMethod_txx
#define __BSplineImageToImageRegistrationMethod_txx

#include "itkBSplineImageToImageRegistrationMethod.h"

#include "itkBSplineResampleImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"
#include "itkBSplineDecompositionImageFilter.h"

namespace itk
{

template< class TImage >
BSplineImageToImageRegistrationMethod< TImage >
::BSplineImageToImageRegistrationMethod( void )
{
  m_NumberOfControlPoints = 10;
  this->SetOptimizationMethodEnum( Superclass::MULTIRESOLUTION_OPTIMIZATION );
  this->SetTransformMethodEnum( Superclass::BSPLINE_TRANSFORM );
}

template< class TImage >
BSplineImageToImageRegistrationMethod< TImage >
::~BSplineImageToImageRegistrationMethod( void )
{
}

template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::ComputeGridRegion( int numberOfControlPoints,
                 typename TransformType::RegionType::SizeType & gridSize,
                 typename TransformType::SpacingType & gridSpacing,
                 typename TransformType::OriginType & gridOrigin,
                 typename TransformType::DirectionType & gridDirection)
{
  if(numberOfControlPoints < 2)
    {
    itkWarningMacro(<< "ComputeGridRegion: numberOfControlPoints=1; changing to 2.");
    numberOfControlPoints = 2;
    }

  typename TransformType::RegionType::SizeType gridSizeOnImage;
  typename TransformType::RegionType::SizeType gridBorderSize;

  typename TImage::SizeType fixedImageSize = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();

  gridSpacing   = this->GetFixedImage()->GetSpacing();

  double scale = (fixedImageSize[0] * gridSpacing[0]) / numberOfControlPoints;
  gridSizeOnImage[0] = numberOfControlPoints;
  for(unsigned int i=1; i<ImageDimension; i++)
    {
    gridSizeOnImage[i] = (int)((fixedImageSize[i] * gridSpacing[i]) / scale);
    if( gridSizeOnImage[i] < 2 )
      {
      gridSizeOnImage[i] = 2;
      }
    }
  gridBorderSize.Fill( 3 );  // Border for spline order = 3 ( 1 lower, 2 upper )

  gridSize = gridSizeOnImage + gridBorderSize;

  gridOrigin    = this->GetFixedImage()->GetOrigin();
  gridDirection    = this->GetFixedImage()->GetDirection();

  for(unsigned int r=0; r<ImageDimension; r++)
    {
    gridSpacing[r] *= floor( static_cast<double>(fixedImageSize[r] - 1)  /
                         static_cast<double>(gridSizeOnImage[r] - 1) );
    }

  typename TransformType::SpacingType gridOriginOffset;
  gridOriginOffset = gridDirection * gridSpacing;

  gridOrigin = gridOrigin - gridOriginOffset;

  std::cout << "gridSize = " << gridSize << std::endl;
  std::cout << "gridSpacing = " << gridSpacing << std::endl;
  std::cout << "gridOrigin = " << gridOrigin << std::endl;
  std::cout << "gridDirection = " << gridDirection << std::endl;
}

 
template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::Update( void )
{
  this->SetTransform( BSplineTransformType::New() );

  typename TransformType::RegionType gridRegion;
  typename TransformType::RegionType::SizeType gridSize;
  typename TransformType::SpacingType  gridSpacing;
  typename TransformType::OriginType  gridOrigin;
  typename TransformType::DirectionType  gridDirection;

  this->ComputeGridRegion( m_NumberOfControlPoints,
                           gridSize, gridSpacing, gridOrigin,
                           gridDirection);

  gridRegion.SetSize( gridSize );

  this->GetTypedTransform()->SetGridRegion( gridRegion );
  this->GetTypedTransform()->SetGridSpacing( gridSpacing );
  this->GetTypedTransform()->SetGridOrigin( gridOrigin );
  this->GetTypedTransform()->SetGridDirection( gridDirection );

  const unsigned int numberOfParameters =
               this->GetTypedTransform()->GetNumberOfParameters();

  if( numberOfParameters != this->GetInitialTransformParameters().GetSize() )
    {
    typename Superclass::TransformParametersType params( numberOfParameters );
    params.Fill( 0.0 );
    this->GetTypedTransform()->SetParametersByValue( params );
  
    this->SetInitialTransformParameters( this->GetTypedTransform()->GetParameters() );
    }

  this->SetInitialTransformFixedParameters( this->GetTypedTransform()->GetFixedParameters() );
  
  typename Superclass::TransformParametersType params( numberOfParameters );
  typename TImage::SizeType fixedImageSize = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();
  typename TransformType::SpacingType spacing   = this->GetFixedImage()->GetSpacing();
  double scale = 10.0 / ((fixedImageSize[0] * spacing[0]) / m_NumberOfControlPoints);
  params.Fill( scale );
  this->SetTransformParametersScales( params );

  Superclass::Update();

}

template< class TImage >
typename BSplineImageToImageRegistrationMethod< TImage >::TransformType *
BSplineImageToImageRegistrationMethod< TImage >
::GetTypedTransform( void )
{
  return static_cast< TransformType  * >( Superclass::GetTransform() );
}

template< class TImage >
typename BSplineImageToImageRegistrationMethod< TImage >::TransformType::Pointer
BSplineImageToImageRegistrationMethod< TImage >
::GetBSplineTransform( void )
{
  typename BSplineTransformType::Pointer trans = BSplineTransformType::New();
  
  trans->SetFixedParameters( this->GetTypedTransform()->GetFixedParameters() );
  trans->SetParametersByValue( this->GetTypedTransform()->GetParameters() );

  return trans;
}

template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::ResampleControlGrid(int numberOfControlPoints,
                      ParametersType & parameters )
{
  typename TransformType::RegionType::SizeType gridSize;
  typename TransformType::SpacingType  gridSpacing;
  typename TransformType::OriginType  gridOrigin;
  typename TransformType::DirectionType  gridDirection;

  this->ComputeGridRegion( numberOfControlPoints,
                           gridSize, gridSpacing, gridOrigin,
                           gridDirection);

  int numberOfParameters = gridSize[0];
  for(unsigned int i=1; i<ImageDimension; i++)
    {
    numberOfParameters *= gridSize[i];
    }
  numberOfParameters *= ImageDimension;

  parameters.SetSize( numberOfParameters );
    
  int parameterCounter = 0;
  for( unsigned int k = 0; k < ImageDimension; k++ )
    {
    typedef typename BSplineTransformType::ImageType  ParametersImageType;

    typedef ResampleImageFilter< ParametersImageType, ParametersImageType> 
                                                      ResamplerType;
    typename ResamplerType::Pointer upsampler = ResamplerType::New();
 
    typedef BSplineResampleImageFunction< ParametersImageType, double > 
                                                      FunctionType;
    typename FunctionType::Pointer function = FunctionType::New();
 
    typedef IdentityTransform< double, ImageDimension > 
                                                      IdentityTransformType;
    typename IdentityTransformType::Pointer identity = 
                                               IdentityTransformType::New();
 
    upsampler->SetInput( this->GetTypedTransform()
                             ->GetCoefficientImage()[k] );
    upsampler->SetInterpolator( function );
    upsampler->SetTransform( identity );
    upsampler->SetSize( gridSize );
    upsampler->SetOutputSpacing( gridSpacing );
    upsampler->SetOutputOrigin( gridOrigin );
    upsampler->SetOutputDirection( gridDirection );
    try
      {
      upsampler->Update();
      }
    catch( itk::ExceptionObject & excep )
      {
      std::cout << "Exception in upsampler: " << excep << std::endl;
      }
    catch( ... )
      {
      std::cout << "Uncaught exception in upsampler" << std::endl;
      }
 
    typedef BSplineDecompositionImageFilter< ParametersImageType,
                                             ParametersImageType >
                                                 DecompositionType;
    typename DecompositionType::Pointer decomposition =
                                          DecompositionType::New();
 
    decomposition->SetSplineOrder( 3 );
    decomposition->SetInput( upsampler->GetOutput() );
    try
      {
      decomposition->Update();
      }
    catch( itk::ExceptionObject & excep )
      {
      std::cout << "Exception in decomposition: " << excep << std::endl;
      }
    catch( ... )
      {
      std::cout << "Uncaught exception in decomposition" << std::endl;
      }
 
    typename ParametersImageType::Pointer newCoefficients =
                                        decomposition->GetOutput();
 
    // copy the coefficients into the parameter array
    typedef ImageRegionIterator<ParametersImageType> Iterator;
    Iterator it( newCoefficients, 
                 newCoefficients->GetLargestPossibleRegion() );
    while ( !it.IsAtEnd() )
      {
      parameters[ parameterCounter++ ] = it.Get();
      ++it;
      }
    }
}



template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}

};

#endif 
