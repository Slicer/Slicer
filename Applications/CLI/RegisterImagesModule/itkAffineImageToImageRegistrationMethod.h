/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: AffineRegistrator.h,v $
  Language:  C++
  Date:      $Date: 2006/11/06 14:39:34 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __AffineImageToImageRegistrationMethod_h
#define __AffineImageToImageRegistrationMethod_h

#include "itkOrientedImage.h"
#include "itkAffineTransform.h"

#include "itkOptimizedImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
class AffineImageToImageRegistrationMethod 
: public OptimizedImageToImageRegistrationMethod< TImage >
{

  public:

    typedef AffineImageToImageRegistrationMethod               Self;
    typedef OptimizedImageToImageRegistrationMethod< TImage >  Superclass;
    typedef SmartPointer< Self >                               Pointer;
    typedef SmartPointer< const Self >                         ConstPointer;

    itkTypeMacro( AffineImageToImageRegistrationMethod,
                  OptimizedImageToImageRegistrationMethod );

    itkNewMacro( Self );

    itkStaticConstMacro( ImageDimension, unsigned int,
                         TImage::ImageDimension );


    //
    // Typedefs from Superclass
    //

    // Overrides the superclass' TransformType typedef
    typedef AffineTransform< double, itkGetStaticConstMacro( ImageDimension ) >
                                                         AffineTransformType;
    typedef typename AffineTransformType::Pointer        AffineTransformPointer;                                                  
    typedef AffineTransformType                          TransformType;

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
     * and SetInitialTransformFixedParameters(). The method below facilitates to 
     * use the AffineTransform returned by the InitialImageToImageRegistrationMethod 
     * to directly initialize this rigid registration method. 
     */
    void SetInitialTransformParametersFromAffineTransform( const AffineTransformType * affine );

  protected:

    AffineImageToImageRegistrationMethod( void );
    virtual ~AffineImageToImageRegistrationMethod( void );

    void PrintSelf( std::ostream & os, Indent indent ) const;
         
  private:

    AffineImageToImageRegistrationMethod( const Self & );  // Purposely not implemented
    void operator = ( const Self & );                     // Purposely not implemented

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAffineImageToImageRegistrationMethod.txx"
#endif

#endif //__ImageToImageRegistrationMethod_h

