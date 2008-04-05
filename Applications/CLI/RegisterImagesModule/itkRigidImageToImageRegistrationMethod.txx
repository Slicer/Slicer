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


#ifndef __RigidImageToImageRegistrationMethod_txx
#define __RigidImageToImageRegistrationMethod_txx

#include "itkRigidImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
RigidImageToImageRegistrationMethod< TImage >
::RigidImageToImageRegistrationMethod( void )
{
  if( ImageDimension == 2 )
    {
    typename Rigid2DTransformType::Pointer tmpTrans = Rigid2DTransformType::New();
    this->SetTransform( dynamic_cast< RigidTransformType *>( tmpTrans.GetPointer()) );
    tmpTrans->Register();
    }
  else if( ImageDimension == 3 )
    {
    typename Rigid3DTransformType::Pointer tmpTrans = Rigid3DTransformType::New();
    this->SetTransform( dynamic_cast< RigidTransformType *>( tmpTrans.GetPointer()) );
    tmpTrans->Register();
    }
  else
    {
    std::cerr << "ERROR: Rigid registration only supported for 2D and 4D images." << std::endl;
    }

  this->GetTypedTransform()->SetIdentity();

  this->SetInitialTransformParameters( this->GetTypedTransform()->GetParameters() );
  this->SetLastTransformParameters( this->GetTypedTransform()->GetParameters() );

  typename Superclass::TransformParametersScalesType scales;
  scales.set_size( this->GetTypedTransform()->GetNumberOfParameters() );
  if( ImageDimension == 2 )
    {
    scales[0] = 0.25;
    scales[1] = 10;
    scales[2] = 10;
    }
  else if( ImageDimension == 3 )
    {
    scales[0] = 0.25;
    scales[1] = 0.25;
    scales[2] = 0.25;
    scales[3] = 10;
    scales[4] = 10;
    scales[5] = 10;
    }

  this->SetTransformParametersScales( scales );

  this->SetTransformMethodEnum( Superclass::RIGID_TRANSFORM );
}

template< class TImage >
RigidImageToImageRegistrationMethod< TImage >
::~RigidImageToImageRegistrationMethod( void )
{
}

template< class TImage >
typename RigidImageToImageRegistrationMethod< TImage >::TransformType *
RigidImageToImageRegistrationMethod< TImage >
::GetTypedTransform( void )
{
  return static_cast< TransformType  * >( Superclass::GetTransform() );
}

template< class TImage >
typename RigidImageToImageRegistrationMethod< TImage >::AffineTransformType::Pointer
RigidImageToImageRegistrationMethod< TImage >
::GetAffineTransform( void )
{   
  typename AffineTransformType::Pointer trans = AffineTransformType::New();

  trans->SetIdentity();
  trans->SetCenter( this->GetTypedTransform()->GetCenter() );
  trans->SetMatrix( this->GetTypedTransform()->GetMatrix() );
  trans->SetOffset( this->GetTypedTransform()->GetOffset() );

  return trans;
}   

template< class TImage >
void
RigidImageToImageRegistrationMethod< TImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf(os, indent);
}

};

#endif 
