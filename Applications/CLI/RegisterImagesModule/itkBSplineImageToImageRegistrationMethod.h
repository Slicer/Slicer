/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: BSplineRegistrator.h,v $
  Language:  C++
  Date:      $Date: 2006/11/06 14:39:34 $
  Version:   $Revision: 1.15 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __BSplineImageToImageRegistrationMethod_h
#define __BSplineImageToImageRegistrationMethod_h

#include "itkImage.h"
#include "itkBSplineDeformableTransform.h"

#include "itkImageToImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
class BSplineImageToImageRegistrationMethod 
: public OptimizedImageToImageRegistrationMethod< TImage >
{

  public:

    typedef BSplineImageToImageRegistrationMethod              Self;
    typedef OptimizedImageToImageRegistrationMethod< TImage >  Superclass;
    typedef SmartPointer< Self >                               Pointer;
    typedef SmartPointer< const Self >                         ConstPointer;

    itkTypeMacro( BSplineImageToImageRegistrationMethod,
                  OptimizedImageToImageRegistrationMethod );

    itkNewMacro( Self );

    //
    // Typedefs from Superclass
    //

    // Overrides the superclass' TransformType typedef
    typedef BSplineDeformableTransform< double,
                                        itkGetStaticConstMacro( ImageDimension ),
                                        itkGetStaticConstMacro( ImageDimension ) >
                                                  BSplineTransformType;

    typedef BSplineTransformType                  TransformType;

    //
    // Methods from Superclass
    //

    void Update( void );

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

    itkSetMacro( NumberOfControlPoints, unsigned int );
    itkGetConstMacro( NumberOfControlPoints, unsigned int );

    typename TransformType::Pointer GetBSplineTransform( void );

  protected:

    BSplineImageToImageRegistrationMethod( void );
    virtual ~BSplineImageToImageRegistrationMethod( void );

    void PrintSelf( std::ostream & os, Indent indent ) const;
         
  private:

    BSplineImageToImageRegistrationMethod( const Self & );  // Purposely not implemented
    void operator = ( const Self & );                     // Purposely not implemented

    unsigned int m_NumberOfControlPoints;

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkBSplineImageToImageRegistrationMethod.txx"
#endif

#endif //__ImageToImageRegistrationMethod_h

