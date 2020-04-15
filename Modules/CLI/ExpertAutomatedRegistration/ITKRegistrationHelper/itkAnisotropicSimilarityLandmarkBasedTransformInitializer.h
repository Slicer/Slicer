/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkAnisotropicSimilarityLandmarkBasedTransformInitializer.h,v $
  Language:  C++
  Date:      $Date: 2008-06-26 13:50:49 $
  Version:   $Revision: 1.7 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkAnisotropicSimilarityLandmarkBasedTransformInitializer_h
#define itkAnisotropicSimilarityLandmarkBasedTransformInitializer_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkAnisotropicSimilarity3DTransform.h"
#include "itkRigid2DTransform.h"
#include <vector>
#include <iostream>

namespace itk
{

/** \class AnisotropicSimilarityLandmarkBasedTransformInitializer
 * \brief AnisotropicSimilarityLandmarkBasedTransformInitializer is a helper class intended to
 * The class computes the transform that aligns the fixed and moving images
 * given a set of landmarks. The class is templated over the Transform type.
 *    The transform computed gives the best fit transform that maps the fixed
 * and moving images in a least squares sense. The indices are taken to
 * correspond, so point 1 in the first set will get mapped close to point
 * 1 in the second set, etc. An equal number of fixed and moving landmarks
 * need to be specified using SetFixedLandmarks() SetMovingLandmarks().
 * Any number of landmarks may be specified.
 * Call InitializeTransform() to initialize the transform.
 *
 * Currently, the  following transforms are supported by the class:
 *    AnisotropicSimilarity3DTransform
 *
 * The class is based in part on Hybrid/vtkLandmarkTransform originally
 * implemented in python by David G. Gobbi.
 *
 * The solution is based on
 * Berthold K. P. Horn (1987), "Closed-form solution of absolute orientation
 * using unit quaternions,"
 * http://people.csail.mit.edu/bkph/papers/Absolute_Orientation.pdf
 *
 *
 * \ingroup Transforms
 */
template <class TTransform,
          class TFixedImage,
          class TMovingImage>
class AnisotropicSimilarityLandmarkBasedTransformInitializer :
  public Object
{
public:
  /** Standard class typedefs. */
  using Self = AnisotropicSimilarityLandmarkBasedTransformInitializer<TTransform, TFixedImage, TMovingImage>;
  using Superclass = Object;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** New macro for creation of through a Smart Pointer. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( AnisotropicSimilarityLandmarkBasedTransformInitializer, Object );

  /** Type of the transform to initialize */
  using TransformType = TTransform;
  using TransformPointer = typename TransformType::Pointer;

  /** Dimension of parameters. */
  itkStaticConstMacro(InputSpaceDimension, unsigned int, TransformType::InputSpaceDimension);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, TransformType::OutputSpaceDimension);

  /** Set the transform to be initialized */
  itkSetObjectMacro( Transform,   TransformType   );

  /** Image Types to use in the initialization of the transform */
  using FixedImageType = TFixedImage;
  using MovingImageType = TMovingImage;

  using FixedImagePointer = typename FixedImageType::ConstPointer;
  using MovingImagePointer = typename MovingImageType::ConstPointer;

  /** \deprecated
   * Set the fixed image.
   * The method really doesn't do anything. The goal of this class is to compute
   * the optimal transform, for the templated TransformType between the fixed
   * and moving image grid, given a set of landmarks. Nothing is done with the
   * images themselves. The method will therefore be deprecated and removed */
  void SetFixedImage( const FixedImageType * image )
  {
    this->m_FixedImage = image;
    itkLegacyBodyMacro( SetFixedImage, 2.2 );
  }

  /** \deprecated
   * Set the moving image.
   * The method really doesn't do anything. The goal of this class is to compute
   * the optimal transform, for the templated TransformType between the fixed
   * and moving image grid, given a set of landmarks. Nothing is done with the
   * images themselves. The method will therefore be deprecated and removed. */
  void SetMovingImage( const MovingImageType * image )
  {
    this->m_MovingImage = image;
    itkLegacyBodyMacro( SetMovingImage, 2.2 );
  }

  /** Determine the image dimension. */
  itkStaticConstMacro(ImageDimension, unsigned int, FixedImageType::ImageDimension );

  /** Convenience typedefs */
  using InputPointType = typename TransformType::InputPointType;
  using OutputVectorType = typename TransformType::OutputVectorType;
  using LandmarkPointType = Point<double, (Self::ImageDimension)>;
  using LandmarkPointContainer = std::vector<LandmarkPointType>;
  using PointsContainerConstIterator = typename LandmarkPointContainer::const_iterator;
  using ParametersType = typename TransformType::ParametersType;
  using ParameterValueType = typename ParametersType::ValueType;

  /** Set the Fixed landmark point containers */
  void SetFixedLandmarks(const LandmarkPointContainer & fixedLandmarks)
  {
    this->m_FixedLandmarks = fixedLandmarks;
  }

  /** Set the Moving landmark point containers */
  void SetMovingLandmarks(const LandmarkPointContainer & movingLandmarks)
  {
    this->m_MovingLandmarks = movingLandmarks;
  }

  /**  Supported Transform typedefs */
  using AnisotropicSimilarity3DTransformType = AnisotropicSimilarity3DTransform<ParameterValueType>;
  using Rigid2DTransformType = Rigid2DTransform<ParameterValueType>;

  /** Initialize the transform from the landmarks */
  virtual void InitializeTransform();

protected:
  AnisotropicSimilarityLandmarkBasedTransformInitializer();
  ~AnisotropicSimilarityLandmarkBasedTransformInitializer() override = default;

  void PrintSelf(std::ostream & os, Indent indent) const override;

  // Supported Transform types
  using InputTransformType = enum
    {
    AnisotropicSimilarity3Dtransform = 1,
    Rigid2Dtransfrom,
    Else
    };
private:
  AnisotropicSimilarityLandmarkBasedTransformInitializer(const Self &); // purposely not implemented
  void operator=(const Self &);                                         // purposely not implemented

  FixedImagePointer  m_FixedImage;
  MovingImagePointer m_MovingImage;

  LandmarkPointContainer m_FixedLandmarks;
  LandmarkPointContainer m_MovingLandmarks;

  TransformPointer m_Transform;

}; // class LandmarkBasedTransformInitializer

}  // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAnisotropicSimilarityLandmarkBasedTransformInitializer.txx"
#endif

#endif /* itkAnisotropicSimilarityLandmarkBasedTransformInitializer_h */
