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

#ifndef __ImageToImageRegistrationMethod_txx
#define __ImageToImageRegistrationMethod_txx

#include "itkImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
ImageToImageRegistrationMethod< TImage >
::ImageToImageRegistrationMethod( void )
{
  this->SetNumberOfRequiredOutputs( 1 ); // the transform

  m_Transform = 0;
  typename TransformOutputType::Pointer transformDecorator = 
                                      static_cast<  TransformOutputType * >
                                        ( this->MakeOutput(0).GetPointer() );

  this->ProcessObject::SetNthOutput( 0, transformDecorator.GetPointer() );

  m_RegistrationNumberOfThreads = this->GetNumberOfThreads();
  this->GetMultiThreader()->SetNumberOfThreads( m_RegistrationNumberOfThreads );

  m_FixedImage = 0;
  m_MovingImage = 0;
  m_FixedImageMaskObject = 0;
  m_MovingImageMaskObject = 0;
  m_Observer = 0;

}

template< class TImage >
ImageToImageRegistrationMethod< TImage >
::~ImageToImageRegistrationMethod( void )
{
}

template< class TImage >
void
ImageToImageRegistrationMethod< TImage >
::SetFixedImage( const ImageType * fixedImage )
{
  if( this->m_FixedImage.GetPointer() != fixedImage )
    {
    this->m_FixedImage = fixedImage;

    this->ProcessObject::SetNthInput(0,
                                     const_cast< ImageType * >( fixedImage ) );
    this->Modified();
    }
}

template< class TImage >
void
ImageToImageRegistrationMethod< TImage >
::SetMovingImage( const ImageType * fixedImage )
{
  if( this->m_MovingImage.GetPointer() != fixedImage )
    {
    this->m_MovingImage = fixedImage;

    this->ProcessObject::SetNthInput(1,
                                     const_cast< ImageType * >( fixedImage ) );
    this->Modified();
    }
}

template< class TImage >
const typename ImageToImageRegistrationMethod< TImage >::TransformOutputType *
ImageToImageRegistrationMethod< TImage >
::GetOutput( ) const
{
  return static_cast< const TransformOutputType * >( this->ProcessObject::GetOutput( 0 ) );
}

template< class TImage >
DataObject::Pointer
ImageToImageRegistrationMethod< TImage >
::MakeOutput( unsigned int idx ) 
{
  switch ( idx )
    {
    case 0:
      return static_cast< DataObject * >( 
               TransformOutputType::New().GetPointer() );
      break;
    default:
      itkExceptionMacro( 
               "MakeOutput request for an output number larger than the expected number of outputs" );
      return 0;
    }
}

template< class TImage >
unsigned long
ImageToImageRegistrationMethod< TImage >
::GetMTime( void ) const
{
  unsigned long mtime = Superclass::GetMTime();
  unsigned long m;

  if( m_Transform.IsNotNull() )
    {
    m = m_Transform->GetMTime();
    mtime = (m > mtime ? m : mtime);
    }

  if( m_FixedImage.IsNotNull() )
    {
    m = m_FixedImage->GetMTime();
    mtime = (m > mtime ? m : mtime);
    }

  if( m_FixedImageMaskObject.IsNotNull() )
    {
    m = m_FixedImageMaskObject->GetMTime();
    mtime = (m > mtime ? m : mtime);
    }

  if( m_MovingImage.IsNotNull() )
    {
    m = m_MovingImage->GetMTime();
    mtime = (m > mtime ? m : mtime);
    }

  if( m_MovingImageMaskObject.IsNotNull() )
    {
    m = m_MovingImageMaskObject->GetMTime();
    mtime = (m > mtime ? m : mtime);
    }

  return mtime;
}

template< class TImage >
void
ImageToImageRegistrationMethod< TImage >
::Initialize( void )
{
  this->GetMultiThreader()->SetNumberOfThreads( m_RegistrationNumberOfThreads );

  if( m_Transform.IsNull() )
    {
    itkExceptionMacro( << "Transform is not set" );
    }

  if( m_FixedImage.IsNull() )
    {
    itkExceptionMacro( << "Fixed image is not set" );
    }

  if( m_MovingImage.IsNull() )
    {
    itkExceptionMacro( << "Moving image is not set" );
    }

  TransformOutputType * transformOutput =
    static_cast< TransformOutputType * >( this->ProcessObject::GetOutput( 0 ) );

  transformOutput->Set( m_Transform.GetPointer() );
}

template< class TImage >
void
ImageToImageRegistrationMethod< TImage >
::GenerateData( void )
{
  this->Update();
}

template< class TImage >
void
ImageToImageRegistrationMethod< TImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Number of threads = " << m_RegistrationNumberOfThreads
     << std::endl;
  if( m_Transform.IsNotNull() )
    {
    os << indent << "Transform = " << m_Transform << std::endl;
    }
  else
    {
    os << indent << "Transform = 0" << std::endl;
    }

  if( m_Observer.IsNotNull() )
    {
    os << indent << "Observer = " << m_Observer << std::endl;
    }
  else
    {
    os << indent << "Observer = 0" << std::endl;
    }

  if( m_FixedImage.IsNotNull() )
    {
    os << indent << "Fixed Image = " << m_FixedImage << std::endl;
    }
  else
    {
    os << indent << "Fixed Image = 0" << std::endl;
    }

  if( m_MovingImage.IsNotNull() )
    {
    os << indent << "Moving Image = " << m_FixedImage << std::endl;
    }
  else
    {
    os << indent << "Moving Image = 0" << std::endl;
    }

  if( m_FixedImageMaskObject.IsNotNull() )
    {
    os << indent << "Fixed Image Mask Object = " << m_FixedImageMaskObject 
                 << std::endl;
    }
  else
    {
    os << indent << "Fixed image mask = 0" << std::endl;
    }

  if( m_MovingImageMaskObject.IsNotNull() )
    {
    os << indent << "Moving Image Mask Object = " << m_MovingImageMaskObject 
                 << std::endl;
    }
  else
    {
    os << indent << "Moving image mask = 0" << std::endl;
    }

}


};

#endif 
