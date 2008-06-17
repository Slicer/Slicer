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


#ifndef __AffineImageToImageRegistrationMethod_txx
#define __AffineImageToImageRegistrationMethod_txx

#include "itkAffineImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
AffineImageToImageRegistrationMethod< TImage >
::AffineImageToImageRegistrationMethod( void )
{
  this->SetTransform( AffineTransformType::New() );
  this->GetTypedTransform()->SetIdentity();

  this->SetInitialTransformParameters( this->GetTypedTransform()
                                           ->GetParameters() );
  this->SetInitialTransformFixedParameters( this->GetTypedTransform()
                                                ->GetFixedParameters() );

  typename Superclass::TransformParametersScalesType scales;
  scales.set_size( this->GetTypedTransform()->GetNumberOfParameters() );
  if( scales.size() != ImageDimension * (ImageDimension + 1) )
    {
    std::cerr << "ERROR: number of parameters not standard for affine transform"
              << std::endl;
    }
  unsigned int scaleNum = 0;
  for(int d1=0; d1<ImageDimension; d1++)
    {
    for(int d2=0; d2<ImageDimension; d2++)
      {
      if(d1 == d2)
        {
        scales[scaleNum] = 100;
        }
      else
        {
        scales[scaleNum] = 1000;
        }
      ++scaleNum;
      }
    }
  for(int d1=0; d1<ImageDimension; d1++)
    {
    scales[scaleNum] = 1;
    ++scaleNum;
    }
  this->SetTransformParametersScales( scales );

  this->SetTransformMethodEnum( Superclass::AFFINE_TRANSFORM );
}

template< class TImage >
AffineImageToImageRegistrationMethod< TImage >
::~AffineImageToImageRegistrationMethod( void )
{
}

template< class TImage >
typename AffineImageToImageRegistrationMethod< TImage >::TransformType *
AffineImageToImageRegistrationMethod< TImage >
::GetTypedTransform( void )
{
  return static_cast< TransformType  * >( Superclass::GetTransform() );
}

template< class TImage >
const typename AffineImageToImageRegistrationMethod< TImage >::TransformType *
AffineImageToImageRegistrationMethod< TImage >
::GetTypedTransform( void ) const
{
  return static_cast< const TransformType  * >( Superclass::GetTransform() );
}

template< class TImage >
typename AffineImageToImageRegistrationMethod< TImage >::AffineTransformPointer
AffineImageToImageRegistrationMethod< TImage >
::GetAffineTransform( void ) const
{   
  AffineTransformPointer trans = AffineTransformType::New();
 
  const TransformType * typedTransform = this->GetTypedTransform();

  trans->SetIdentity();
  trans->SetCenter( typedTransform->GetCenter() );
  trans->SetMatrix( typedTransform->GetMatrix() );
  trans->SetOffset( typedTransform->GetOffset() );

  return trans;
}   


template< class TImage >
void 
AffineImageToImageRegistrationMethod< TImage >
::SetInitialTransformParametersFromAffineTransform( const AffineTransformType * affine )
{
  this->SetInitialTransformFixedParameters( affine->GetFixedParameters() );
  this->SetInitialTransformParameters( affine->GetParameters() );
}


template< class TImage >
void
AffineImageToImageRegistrationMethod< TImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}

};

#endif 
