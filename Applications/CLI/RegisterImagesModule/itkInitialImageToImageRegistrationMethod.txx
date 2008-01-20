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


#ifndef __InitialImageToImageRegistrationMethod_txx
#define __InitialImageToImageRegistrationMethod_txx

#include "itkInitialImageToImageRegistrationMethod.h"

#include "itkImageMomentsCalculator.h"

namespace itk
{

template< class TImage >
InitialImageToImageRegistrationMethod< TImage >
::InitialImageToImageRegistrationMethod( void )
{
  this->SetTransform( TransformType::New() );
  this->GetTypedTransform()->SetIdentity();

  m_NumberOfMoments = 0 ;
}

template< class TImage >
InitialImageToImageRegistrationMethod< TImage >
::~InitialImageToImageRegistrationMethod( void )
{
}

template< class TImage >
void
InitialImageToImageRegistrationMethod< TImage >
::Update( void )
{
  Superclass::Update();

  typedef ImageMomentsCalculator< TImage > MomentsCalculatorType;

  typename MomentsCalculatorType::AffineTransformType::Pointer newTransform;
  newTransform = MomentsCalculatorType::AffineTransformType::New();
  newTransform->SetIdentity();

  if(m_NumberOfMoments == 0)
    {
    typename TImage::SizeType    size;

    //  Fixed image info
    typename TImage::IndexType   fixedCenterIndex;
    Point<double, 3>             fixedCenterPoint;

    size = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();

    fixedCenterIndex[0] = size[0]/2;
    fixedCenterIndex[1] = size[1]/2;
    fixedCenterIndex[2] = size[2]/2;
    this->GetFixedImage()->TransformIndexToPhysicalPoint(fixedCenterIndex,
                                                         fixedCenterPoint);

    //  Moving image info
    typename TImage::IndexType   movingCenterIndex;
    Point<double, 3>             movingCenterPoint;

    size = this->GetMovingImage()->GetLargestPossibleRegion().GetSize();

    movingCenterIndex[0] = size[0]/2;
    movingCenterIndex[1] = size[1]/2;
    movingCenterIndex[2] = size[2]/2;
    this->GetMovingImage()->TransformIndexToPhysicalPoint(movingCenterIndex,
                                                          movingCenterPoint);

    //  Compute alignment
    typename TransformType::OffsetType   offset;
    offset = movingCenterPoint - fixedCenterPoint;

    newTransform->SetCenter(movingCenterPoint);
    newTransform->SetOffset(offset);
    }
  else 
    {
    typedef ImageMomentsCalculator< TImage > MomentsCalculatorType;

    typename MomentsCalculatorType::Pointer momCalc;
    momCalc = MomentsCalculatorType::New();

    momCalc->SetImage( this->GetFixedImage() );
    if( this->GetUseMasks() )
      {
      if( this->GetFixedImageMaskObject() )
        {
        momCalc->SetSpatialObjectMask( this->GetFixedImageMaskObject() );
        }
      }
    // HELP: ImageMomentsCalculator isn't multi-threaded :(
    //momCalc->SetNumberOfThreads( this->GetRegistrationNumberOfThreads() );
    momCalc->Compute();

    typename MomentsCalculatorType::AffineTransformType::Pointer 
          fixedImageAxesTransform;
    fixedImageAxesTransform = 
          momCalc->GetPhysicalAxesToPrincipalAxesTransform();

    typename TransformType::InputPointType fixedImageCenterOfMass;
    for(unsigned int i=0; i<this->ImageDimension; i++)
      {
      fixedImageCenterOfMass[i] = momCalc->GetCenterOfGravity()[i];
      }

    momCalc->SetImage( this->GetMovingImage() );
    if( this->GetUseMasks() )
      {
      if( this->GetMovingImageMaskObject() )
        {
        momCalc->SetSpatialObjectMask( this->GetMovingImageMaskObject() );
        }
      }
    momCalc->Compute();

    typename MomentsCalculatorType::AffineTransformType::Pointer 
          movingImageAxesTransform;
    movingImageAxesTransform = 
          momCalc->GetPrincipalAxesToPhysicalAxesTransform();

    typename TransformType::InputPointType movingImageCenterOfMass;
    for(unsigned int i=0; i<this->ImageDimension; i++)
      {
      movingImageCenterOfMass[i] = momCalc->GetCenterOfGravity()[i];
      }

    typename TransformType::OffsetType offset;
    offset = movingImageCenterOfMass - fixedImageCenterOfMass;
    
    if(m_NumberOfMoments == 1) // Centers of mass
      {
      newTransform->SetCenter(movingImageCenterOfMass);
      newTransform->SetOffset(offset);
      }
    else  // m_NumberOfMoments == 2 // Principle axes
      {
      newTransform->SetCenter(fixedImageCenterOfMass);
      newTransform->SetMatrix(fixedImageAxesTransform->GetMatrix());
      newTransform->SetOffset(fixedImageAxesTransform->GetOffset());
      newTransform->Compose(movingImageAxesTransform, true);
      }
    }

  this->SetTransform(newTransform);
}

template< class TImage >
typename InitialImageToImageRegistrationMethod< TImage >::TransformType *
InitialImageToImageRegistrationMethod< TImage >
::GetTypedTransform( void )
{
  return static_cast< TransformType  * >( Superclass::GetTransform() );
}

template< class TImage >
typename InitialImageToImageRegistrationMethod< TImage >::TransformType::Pointer
InitialImageToImageRegistrationMethod< TImage >
::GetAffineTransform( void )
{   
  typename TransformType::Pointer trans = TransformType::New();

  trans->SetIdentity();
  trans->SetCenter( this->GetTypedTransform()->GetCenter() );
  trans->SetMatrix( this->GetTypedTransform()->GetMatrix() );
  trans->SetOffset( this->GetTypedTransform()->GetOffset() );

  return trans;
}   


template< class TImage >
void
InitialImageToImageRegistrationMethod< TImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Number of moments = " << m_NumberOfMoments << std::endl;
}

};

#endif 
