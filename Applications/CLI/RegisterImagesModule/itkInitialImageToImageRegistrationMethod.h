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

#ifndef __InitialImageToImageRegistrationMethod_h
#define __InitialImageToImageRegistrationMethod_h

#include "itkImage.h"
#include "itkCommand.h"

#include "itkImageToImageRegistrationMethod.h"

#include "itkAffineTransform.h"

namespace itk
{

template< class TImage >
class InitialImageToImageRegistrationMethod 
: public ImageToImageRegistrationMethod< TImage >
  {

  public:

    typedef InitialImageToImageRegistrationMethod     Self;
    typedef ImageToImageRegistrationMethod< TImage >  Superclass;
    typedef SmartPointer< Self >                      Pointer;
    typedef SmartPointer< const Self >                ConstPointer;

    itkTypeMacro( InitialImageToImageRegistrationMethod,
                  ImageToImageRegistrationMethod );

    itkNewMacro( Self );

    //
    // Typedefs from Superclass
    //
    itkStaticConstMacro( ImageDimension, unsigned int,
                         TImage::ImageDimension );

    typedef AffineTransform< double, itkGetStaticConstMacro( ImageDimension ) >
                                                      TransformType;

    //
    //  Methods from Superclass
    //
    void    Update();

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

    typename TransformType::Pointer GetAffineTransform( void );

    itkSetMacro( NumberOfMoments, unsigned int );
    itkGetConstMacro( NumberOfMoments, unsigned int );

  protected:

    InitialImageToImageRegistrationMethod( void );
    virtual ~InitialImageToImageRegistrationMethod( void );

    void PrintSelf( std::ostream & os, Indent indent ) const;
         
  private:

    InitialImageToImageRegistrationMethod( const Self & );  // Purposely not implemented
    void operator = ( const Self & );                       // Purposely not implemented

    unsigned int            m_NumberOfMoments;

  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkInitialImageToImageRegistrationMethod.txx"
#endif


#endif //__ImageToImageRegistrationMethod_h

