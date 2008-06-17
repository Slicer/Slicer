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

#ifndef __RigidImageToImageRegistrationMethod_h
#define __RigidImageToImageRegistrationMethod_h

#include "itkImage.h"
#include "itkAffineTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkRigid2DTransform.h"

#include "itkOptimizedImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
class RigidImageToImageRegistrationMethod 
: public OptimizedImageToImageRegistrationMethod< TImage >
{

  public:

    typedef RigidImageToImageRegistrationMethod                Self;
    typedef OptimizedImageToImageRegistrationMethod< TImage >  Superclass;
    typedef SmartPointer< Self >                               Pointer;
    typedef SmartPointer< const Self >                         ConstPointer;

    itkTypeMacro( RigidImageToImageRegistrationMethod,
                  OptimizedImageToImageRegistrationMethod );

    itkNewMacro( Self );

    //
    // Typedefs from Superclass
    //

    itkStaticConstMacro( ImageDimension, unsigned int,
                         TImage::ImageDimension );

    // Overrides the superclass' TransformType typedef
    // We must use MatrixOffsetTransformBase since no itk rigid transform is
    //   templated over ImageDimension.
    typedef MatrixOffsetTransformBase< double,
                                       itkGetStaticConstMacro( ImageDimension ), 
                                       itkGetStaticConstMacro( ImageDimension ) >
                                                 RigidTransformType;
    typedef RigidTransformType                   TransformType;

    //
    //  Custom Typedefs
    //
    typedef Rigid2DTransform< double >           Rigid2DTransformType;
    typedef VersorRigid3DTransform< double >     Rigid3DTransformType;

    typedef AffineTransform< double,
                             itkGetStaticConstMacro( ImageDimension ) >
                                                        AffineTransformType;

    typedef typename AffineTransformType::Pointer       AffineTransformPointer;                                                 

    //
    // Custom Methods
    //

    /**
     * The function performs the casting.  This function should only appear
     *   once in the class hierarchy.  It is provided so that member
     *   functions that exist only in specific transforms (e.g., SetIdentity)
     *   can be called without the caller having to do the casting. 
     **/
    TransformType * GetTypedTransform( void );
    const TransformType * GetTypedTransform( void ) const;

    /**
     * This function creates a new affine transforms that implements the
     * current registration transform.   Provided to help with transform
     * composition. The transform is initialized with the current results
     * available in the GetTypedTransform() method. The returned transform is
     * not a member variable, and therefore, must be received into a
     * SmartPointer to prevent it from being destroyed by depletion of its
     * reference counting.
     **/
    AffineTransformPointer GetAffineTransform( void ) const;


    /** Initialize the transform parameters from an AffineTransform This method
     * is intended as an alternative to calling SetInitialTransformParameters()
     * and SetInitialTransformFixedParameters(). These later methods require
     * you to have a rigid transform at hand, and this is not always the case,
     * specially when a transform initializer is being used. The method below
     * facilitates to use the AffineTransform returned by the
     * InitialImageToImageRegistrationMethod to directly initialize this rigid
     * registration method. The received Affine transform will be approximated
     * to its closest rigid transform by using Polar decomposition.
     */
    void SetInitialTransformParametersFromAffineTransform( const AffineTransformType * affine );

  protected:

    RigidImageToImageRegistrationMethod( void );
    virtual ~RigidImageToImageRegistrationMethod( void );

    void PrintSelf( std::ostream & os, Indent indent ) const;
         
  private:

    RigidImageToImageRegistrationMethod( const Self & );  // Purposely not implemented
    void operator = ( const Self & );                     // Purposely not implemented

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRigidImageToImageRegistrationMethod.txx"
#endif

#endif //__ImageToImageRegistrationMethod_h

