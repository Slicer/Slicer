/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: RigidRegistrator.h,v $
  Language:  C++
  Date:      $Date: 2006/11/06 14:39:34 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkInitialImageToImageRegistrationMethod_h
#define itkInitialImageToImageRegistrationMethod_h

#include "itkImage.h"
#include "itkCommand.h"

#include "itkImageToImageRegistrationMethod.h"

#include "itkAffineTransform.h"

#include "itkAnisotropicSimilarity3DTransform.h"
#include "itkAnisotropicSimilarityLandmarkBasedTransformInitializer.h"

namespace itk
{

template <class TImage>
class InitialImageToImageRegistrationMethod
  : public ImageToImageRegistrationMethod<TImage>
{

public:

  using Self = InitialImageToImageRegistrationMethod<TImage>;
  using Superclass = ImageToImageRegistrationMethod<TImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  itkTypeMacro( InitialImageToImageRegistrationMethod,
                ImageToImageRegistrationMethod );

  itkNewMacro( Self );

  //
  // Typedefs from Superclass
  //
  itkStaticConstMacro( ImageDimension, unsigned int,
                       TImage::ImageDimension );

  using TransformType = AffineTransform<double, (Self::ImageDimension)>;

  using TransformPointer = typename TransformType::Pointer;

  //
  // Local Typedefs
  //
  using LandmarkPointType = Point<double, (Self::ImageDimension)>;
  using LandmarkPointContainer = std::vector<LandmarkPointType>;

  //
  // Custom Methods
  //

  /**
   * The function performs the casting.  This function should only appear
   *   once in the class hierarchy.  It is provided so that member
   *   functions that exist only in specific transforms (e.g., SetIdentity)
   *   can be called without the caller having to do the casting.
   **/
  TransformType* GetTypedTransform();

  const TransformType* GetTypedTransform() const;

  /** This method creates, initializes and returns an Affine transform.  The
   * transform is initialized with the current results available in the
   * GetTypedTransform() method. The returned transform is not a member
   * variable, and therefore, must be received into a SmartPointer to prevent
   * it from being destroyed by depletion of its reference counting. */
  TransformPointer GetAffineTransform() const;

  itkSetMacro( NumberOfMoments, unsigned int );
  itkGetConstMacro( NumberOfMoments, unsigned int );

  itkSetMacro( ComputeCenterOfRotationOnly, bool );
  itkGetConstMacro( ComputeCenterOfRotationOnly, bool );

  itkSetMacro( UseLandmarks, bool );
  itkGetConstMacro( UseLandmarks, bool );

  void SetFixedLandmarks( const LandmarkPointContainer& fixedLandmarks );

  void SetMovingLandmarks( const LandmarkPointContainer& movingLandmarks );

protected:
  InitialImageToImageRegistrationMethod();
  ~InitialImageToImageRegistrationMethod() override;

  void PrintSelf( std::ostream & os, Indent indent ) const override;

  //
  //  Methods from Superclass. Only the GenerateData() method should be
  //  overloaded. The Update() method must not be overloaded.
  //
  void    GenerateData() override;

private:

  InitialImageToImageRegistrationMethod( const Self & );    // Purposely not implemented
  void operator =( const Self & );                          // Purposely not implemented

  unsigned int           m_NumberOfMoments;
  bool                   m_ComputeCenterOfRotationOnly;
  bool                   m_UseLandmarks;
  LandmarkPointContainer m_FixedLandmarks;
  LandmarkPointContainer m_MovingLandmarks;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkInitialImageToImageRegistrationMethod.txx"
#endif

#endif // __ImageToImageRegistrationMethod_h
