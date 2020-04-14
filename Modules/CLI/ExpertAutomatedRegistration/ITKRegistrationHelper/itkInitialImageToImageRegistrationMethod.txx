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

#ifndef itkInitialImageToImageRegistrationMethod_txx
#define itkInitialImageToImageRegistrationMethod_txx

#include "itkInitialImageToImageRegistrationMethod.h"

#include "itkImageRegionMomentsCalculator.h"

namespace itk
{

template <class TImage>
InitialImageToImageRegistrationMethod<TImage>::InitialImageToImageRegistrationMethod()
{
  this->SetTransform( TransformType::New() );
  this->GetTypedTransform()->SetIdentity();

  this->m_NumberOfMoments = 0;

  this->m_ComputeCenterOfRotationOnly = false;

  this->m_UseLandmarks = false;
}

template <class TImage>
InitialImageToImageRegistrationMethod<TImage>::~InitialImageToImageRegistrationMethod() = default;

/** Only the GenerateData() method should be overloaded. The Update() method
 * must not be overloaded */
template <class TImage>
void InitialImageToImageRegistrationMethod<TImage>::GenerateData()
{
  Superclass::GenerateData();

  if( this->m_UseLandmarks )
    {
    typename TransformType::Pointer newTransform;
    newTransform = TransformType::New();
    newTransform->SetIdentity();

    typename TransformType::InputPointType center;
    typename TransformType::MatrixType matrix;
    typename TransformType::OffsetType offset;

    if( TImage::ImageDimension == 3 )
      {
      typedef AnisotropicSimilarity3DTransform<double> LandmarkTransformType;
      typedef AnisotropicSimilarityLandmarkBasedTransformInitializer<LandmarkTransformType,
                                                                     TImage, TImage>
      LandmarkTransformCalculatorType;

      typename LandmarkTransformCalculatorType::Pointer landmarkCalc;
      landmarkCalc = LandmarkTransformCalculatorType::New();
      landmarkCalc->SetFixedLandmarks( m_FixedLandmarks );
      landmarkCalc->SetMovingLandmarks( m_MovingLandmarks );

      typename LandmarkTransformType::Pointer landmarkTransform;
      landmarkTransform = LandmarkTransformType::New();
      landmarkTransform->SetIdentity();
      landmarkCalc->SetTransform(landmarkTransform);
      landmarkCalc->InitializeTransform();
      for( unsigned int i = 0; i < TImage::ImageDimension; i++ )
        {
        center[i] = landmarkTransform->GetCenter()[i];
        offset[i] = landmarkTransform->GetTranslation()[i];
        for( unsigned int j = 0; j < TImage::ImageDimension; j++ )
          {
          matrix(i, j) = landmarkTransform->GetMatrix() (i, j);
          }
        }
      }
    else if( TImage::ImageDimension == 2 )
      {
      typedef Rigid2DTransform<double> LandmarkTransformType;
      typedef AnisotropicSimilarityLandmarkBasedTransformInitializer<LandmarkTransformType,
                                                                     TImage, TImage>
      LandmarkTransformCalculatorType;

      typename LandmarkTransformCalculatorType::Pointer landmarkCalc;
      landmarkCalc = LandmarkTransformCalculatorType::New();
      landmarkCalc->SetFixedLandmarks( m_FixedLandmarks );
      landmarkCalc->SetMovingLandmarks( m_MovingLandmarks );

      typename LandmarkTransformType::Pointer landmarkTransform;
      landmarkTransform = LandmarkTransformType::New();
      landmarkTransform->SetIdentity();
      landmarkCalc->SetTransform(landmarkTransform);
      landmarkCalc->InitializeTransform();
      for( unsigned int i = 0; i < TImage::ImageDimension; i++ )
        {
        center[i] = landmarkTransform->GetCenter()[i];
        offset[i] = landmarkTransform->GetTranslation()[i];
        for( unsigned int j = 0; j < TImage::ImageDimension; j++ )
          {
          matrix(i, j) = landmarkTransform->GetMatrix() (i, j);
          }
        }
      double tf;
      double sizeFixed = 0;
      for( int i = 1; i < (int)m_FixedLandmarks.size(); i++ )
        {
        tf = (m_FixedLandmarks[i][0] - m_FixedLandmarks[i - 1][0]);
        sizeFixed += tf * tf;
        tf = (m_FixedLandmarks[i][1] - m_FixedLandmarks[i - 1][1]);
        sizeFixed += tf * tf;
        }
      sizeFixed = sqrt(sizeFixed);
      double sizeMoving = 0;
      for( int i = 1; i < (int)m_MovingLandmarks.size(); i++ )
        {
        tf = (m_MovingLandmarks[i][0] - m_MovingLandmarks[i - 1][0]);
        sizeMoving += tf * tf;
        tf = (m_MovingLandmarks[i][1] - m_MovingLandmarks[i - 1][1]);
        sizeMoving += tf * tf;
        }
      sizeMoving = sqrt(sizeMoving);
      double scale = sizeMoving / sizeFixed;
      std::cout << "scale = " << scale << std::endl;
      matrix *= scale;
      }
    else
      {
      std::cout << "Image type must be 3 or 2." << std::endl;
      std::cout << "Initialization returning identity." << std::endl;
      }

    newTransform->SetCenter( center );
    newTransform->SetMatrix( matrix );
    newTransform->SetTranslation( offset );
    this->SetTransform(newTransform);

    return;
    }

  typedef ImageRegionMomentsCalculator<TImage> MomentsCalculatorType;

  typename MomentsCalculatorType::AffineTransformType::Pointer newTransform;
  newTransform = MomentsCalculatorType::AffineTransformType::New();
  newTransform->SetIdentity();

  if( this->m_ComputeCenterOfRotationOnly )
    {
    typename TImage::SizeType    size;

    //  Moving image info
    typename TImage::IndexType       movingCenterIndex;
    Point<double, ImageDimension> movingCenterPoint;

    size = this->GetMovingImage()->GetLargestPossibleRegion().GetSize();
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      movingCenterIndex[i] = size[i] / 2;
      }
    this->GetMovingImage()->TransformIndexToPhysicalPoint(movingCenterIndex,
                                                          movingCenterPoint);

    newTransform->SetCenter(movingCenterPoint);
    }
  else if( this->m_NumberOfMoments == 0 )
    {
    typename TImage::SizeType    size;

    //  Fixed image info
    typename TImage::IndexType        fixedCenterIndex;
    Point<double, ImageDimension> fixedCenterPoint;

    size = this->GetFixedImage()->GetLargestPossibleRegion().GetSize();

    if( !this->GetUseRegionOfInterest() )
      {
      std::cout << "Init: Using full image extent" << std::endl;
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        fixedCenterIndex[i] = size[i] / 2;
        }
      this->GetFixedImage()->TransformIndexToPhysicalPoint(fixedCenterIndex,
                                                           fixedCenterPoint);
      }
    else
      {
      std::cout << "Init: Using region of interest" << std::endl;
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        fixedCenterPoint[i] = ( this->GetRegionOfInterestPoint1()[i]
                                + this->GetRegionOfInterestPoint2()[i] ) / 2;
        }
      }

    //  Moving image info
    typename TImage::IndexType       movingCenterIndex;
    Point<double, ImageDimension> movingCenterPoint;

    size = this->GetMovingImage()->GetLargestPossibleRegion().GetSize();
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      movingCenterIndex[i] = size[i] / 2;
      }
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
    typename MomentsCalculatorType::Pointer momCalc;
    momCalc = MomentsCalculatorType::New();

    momCalc->SetImage( this->GetFixedImage() );
    if( this->GetUseFixedImageMaskObject() )
      {
      if( this->GetFixedImageMaskObject() )
        {
        momCalc->SetSpatialObjectMask( this->GetFixedImageMaskObject() );
        }
      }
    if( this->GetUseRegionOfInterest() )
      {
      std::cout << "Init: Region of interest" << std::endl;
      momCalc->SetRegionOfInterest( this->GetRegionOfInterestPoint1(),
                                    this->GetRegionOfInterestPoint2() );
      }
    else
      {
      std::cout << "Init: Using full image extent" << std::endl;
      }

    // HELP: ImageMomentsCalculator isn't multi-threaded :(
    // momCalc->SetNumberOfThreads( this->GetRegistrationNumberOfThreads() );
    try
      {
      momCalc->Compute();
      }
    catch( ... )
      {
      std::cout << "Exception thrown when computing moments of fixed image." << std::endl;
      std::cout << "Initialization returning identity." << std::endl;
      newTransform->SetIdentity();
      this->SetTransform(newTransform);
      return;
      }

    typename MomentsCalculatorType::AffineTransformType::Pointer fixedImageAxesTransform;

    fixedImageAxesTransform = momCalc->GetPhysicalAxesToPrincipalAxesTransform();

    typename TransformType::InputPointType fixedImageCenterOfMass;
    for( unsigned int i = 0; i < this->ImageDimension; i++ )
      {
      fixedImageCenterOfMass[i] = momCalc->GetCenterOfGravity()[i];
      }

    momCalc->SetImage( this->GetMovingImage() );

    if( this->GetUseMovingImageMaskObject() )
      {
      if( this->GetMovingImageMaskObject() )
        {
        momCalc->SetSpatialObjectMask( this->GetMovingImageMaskObject() );
        }
      }

    try
      {
      momCalc->Compute();
      }
    catch( ... )
      {
      std::cout << "Exception thrown when computing moments of moving image." << std::endl;
      std::cout << "Initialization returning identity." << std::endl;
      newTransform->SetIdentity();
      this->SetTransform(newTransform);
      return;
      }

    typename MomentsCalculatorType::AffineTransformType::Pointer
    movingImageAxesTransform;
    movingImageAxesTransform =
      momCalc->GetPrincipalAxesToPhysicalAxesTransform();

    typename TransformType::InputPointType movingImageCenterOfMass;
    for( unsigned int i = 0; i < this->ImageDimension; i++ )
      {
      movingImageCenterOfMass[i] = momCalc->GetCenterOfGravity()[i];
      }

    typename TransformType::OffsetType offset;
    offset = movingImageCenterOfMass - fixedImageCenterOfMass;

    if( this->m_NumberOfMoments == 1 ) // Centers of mass
      {
      newTransform->SetCenter(movingImageCenterOfMass);
      newTransform->SetOffset(offset);
      }
    else  // m_NumberOfMoments == 2 // Principle axes
      {
      newTransform->SetCenter(fixedImageCenterOfMass);
      newTransform->SetMatrix(fixedImageAxesTransform->GetMatrix() );
      newTransform->SetOffset(fixedImageAxesTransform->GetOffset() );
      newTransform->Compose(movingImageAxesTransform, true);
      }
    }

  this->SetTransform(newTransform);
}

template <class TImage>
typename InitialImageToImageRegistrationMethod<TImage>::TransformType*
InitialImageToImageRegistrationMethod<TImage>::GetTypedTransform()
{
  return static_cast<TransformType  *>( Superclass::GetTransform() );
}

template <class TImage>
const typename InitialImageToImageRegistrationMethod<TImage>::TransformType*
InitialImageToImageRegistrationMethod<TImage>::GetTypedTransform() const
{
  return static_cast<const TransformType *>( this->Superclass::GetTransform() );
}

template <class TImage>
typename InitialImageToImageRegistrationMethod<TImage>::TransformPointer
InitialImageToImageRegistrationMethod<TImage>::GetAffineTransform() const
{
  typename TransformType::Pointer trans = TransformType::New();

  const TransformType * typededTransform = this->GetTypedTransform();

  trans->SetIdentity();
  trans->SetCenter( typededTransform->GetCenter() );
  trans->SetMatrix( typededTransform->GetMatrix() );
  trans->SetOffset( typededTransform->GetOffset() );

  return trans;
}

template <class TImage>
void
InitialImageToImageRegistrationMethod<TImage>
::SetFixedLandmarks( const LandmarkPointContainer& fixedLandmarks )
{
  m_FixedLandmarks = fixedLandmarks;
  this->Modified();
}

template <class TImage>
void
InitialImageToImageRegistrationMethod<TImage>
::SetMovingLandmarks( const LandmarkPointContainer& movingLandmarks )
{
  m_MovingLandmarks = movingLandmarks;
  this->Modified();
}

template <class TImage>
void
InitialImageToImageRegistrationMethod<TImage>
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Number of moments = " << this->m_NumberOfMoments << std::endl;

  os << indent << "Compute Center Of Rotation Only = " << this->m_ComputeCenterOfRotationOnly << std::endl;

  os << indent << "Use Landmarks = " << this->m_UseLandmarks << std::endl;
}

}

#endif
