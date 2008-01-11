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

#include "itkImageToImageRegistrationMethod.h"

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

    // Overrides the superclass' TransformType typedef
    typedef VersorRigid3DTransform< double >     RigidTransformType;
    typedef RigidTransformType                   TransformType;

    //
    //  Custom Typedefs
    //
    typedef AffineTransform< double, itkGetStaticConstMacro( ImageDimension ) >
                                                 AffineTransformType;
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

    /**
     * This function creates a new affine transforms that implements the 
     *   current registration transform.   Provided to help with transform
     *   composition
     **/
    typename AffineTransformType::Pointer GetAffineTransform( void );

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

