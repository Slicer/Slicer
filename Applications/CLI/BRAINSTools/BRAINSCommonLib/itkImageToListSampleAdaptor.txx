/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkImageToListSampleAdaptor.txx,v $
  Language:  C++
  Date:      $Date: 2009-05-08 16:55:05 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkImageToListSampleAdaptor_txx
#define __itkImageToListSampleAdaptor_txx

#include "itkImageToListSampleAdaptor.h"

namespace itk { 
namespace Statistics {

template < class TImage>
ImageToListSampleAdaptor< TImage>
::ImageToListSampleAdaptor()
{
  m_Image = 0;
  m_UsePixelContainer = true;
}

template < class TImage>
const typename ImageToListSampleAdaptor< TImage >::MeasurementVectorType&
ImageToListSampleAdaptor< TImage>
::GetMeasurementVector(InstanceIdentifier id) const 
{
  if( m_Image.IsNull() )
    {
    itkExceptionMacro("Image has not been set yet");
    }

  if ( m_UsePixelContainer )
    {
    MeasurementVectorTraits::Assign( m_MeasurementVectorInternal,
                    (*m_PixelContainer)[id]); 
    }
  else
    {
    MeasurementVectorTraits::Assign( m_MeasurementVectorInternal,
                    m_Image->GetPixel( m_Image->ComputeIndex( id ) ) ); 
    }

  return m_MeasurementVectorInternal;
}

/** returns the number of measurement vectors in this container*/
template < class TImage>
typename ImageToListSampleAdaptor< TImage>::InstanceIdentifier
ImageToListSampleAdaptor< TImage>
::Size() const
{
  if( m_Image.IsNull() )
    {
    return 0;  // BIG HACK -- itkSample's SetMeasurementVectorSize is testing Size() Zero for not yet populated with data.
    itkExceptionMacro("Image has not been set yet");
    }

  return m_Image->GetPixelContainer()->Size();
}

template < class TImage>
inline typename ImageToListSampleAdaptor< TImage>::AbsoluteFrequencyType
ImageToListSampleAdaptor< TImage>
::GetFrequency( InstanceIdentifier ) const 
{
  if( m_Image.IsNull() )
    {
    itkExceptionMacro("Image has not been set yet");
    }

  return NumericTraits< AbsoluteFrequencyType >::One;
}


template < class TImage>
void
ImageToListSampleAdaptor< TImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Image: ";
  if ( m_Image.IsNotNull() )
    {
    os << m_Image << std::endl;
    }
  else
    {
    os << "not set." << std::endl;
    }
  os << indent << "UsePixelContainer: "
     << this->GetUsePixelContainer() << std::endl;
}

template < class TImage>
void
ImageToListSampleAdaptor< TImage>
::SetImage(const TImage* image) 
{ 
  m_Image = image; 
  m_PixelContainer = image->GetPixelContainer();
  this->Modified();
}

template < class TImage>
const TImage*
ImageToListSampleAdaptor< TImage>
::GetImage() const
{
  if( m_Image.IsNull() )
    {
    itkExceptionMacro("Image has not been set yet");
    }

  return m_Image.GetPointer(); 
}  

template < class TImage>
typename ImageToListSampleAdaptor< TImage>::TotalAbsoluteFrequencyType
ImageToListSampleAdaptor< TImage>
::GetTotalFrequency() const
{ 
  if( m_Image.IsNull() )
    {
    itkExceptionMacro("Image has not been set yet");
    }

  return this->Size(); 
}

} // end of namespace Statistics 
} // end of namespace itk

#endif
