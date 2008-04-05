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

namespace itk
{

template< class TImage >
BSplineImageToImageRegistrationMethod< TImage >
::BSplineImageToImageRegistrationMethod( void )
{
  m_NumberOfControlPoints = 10;
}

template< class TImage >
BSplineImageToImageRegistrationMethod< TImage >
::~BSplineImageToImageRegistrationMethod( void )
{
}

template< class TImage >
void
BSplineImageToImageRegistrationMethod< TImage >
::Update( void )
{
  this->SetTransform( BSplineTransformType::New() );

  typename TransformType::RegionType bsplineRegion;
  typename TransformType::RegionType::SizeType gridSizeOnImage;
  typename TransformType::RegionType::SizeType gridBorderSize;
  typename TransformType::RegionType::SizeType totalGridSize;

  gridSizeOnImage.Fill( m_NumberOfControlPoints - 3 );  
  gridBorderSize.Fill( 3 );  // Border for spline order = 3 ( 1 lower, 2 upper )

  totalGridSize = gridSizeOnImage + gridBorderSize;

  bsplineRegion.SetSize( totalGridSize );

  typename TransformType::SpacingType spacing   = this->GetFixedImage()->GetSpacing();
  typename TransformType::OriginType  origin    = this->GetFixedImage()->GetOrigin();
  typename TransformType::DirectionType  direction    = this->GetFixedImage()->GetDirection();

  typename TImage::SizeType fixedImageSize = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();

  for(unsigned int r=0; r<3; r++)
    {
    spacing[r] *= floor( static_cast<double>(fixedImageSize[r] - 1)  /
                         static_cast<double>(gridSizeOnImage[r] - 1) );
    origin[r]  -=  spacing[r];
    }

  this->GetTypedTransform()->SetGridRegion( bsplineRegion );
  this->GetTypedTransform()->SetGridSpacing( spacing );
  this->GetTypedTransform()->SetGridOrigin( origin );
  this->GetTypedTransform()->SetGridDirection( direction );

  const unsigned int numberOfParameters =
               this->GetTypedTransform()->GetNumberOfParameters();

  typename Superclass::TransformParametersType params( numberOfParameters );
  params.Fill( 0.0 );
  this->GetTypedTransform()->SetParametersByValue( params );

  this->SetTransformMethodEnum( Superclass::BSPLINE_TRANSFORM );
  this->SetInitialTransformParameters( this->GetTypedTransform()->GetParameters() );

  params.Fill( 1.0f );
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
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}

};

#endif 
