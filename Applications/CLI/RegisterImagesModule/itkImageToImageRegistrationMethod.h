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

#ifndef __ImageToImageRegistrationMethod_h
#define __ImageToImageRegistrationMethod_h

#include "itkImage.h"
#include "itkCommand.h"

#include "itkSpatialObject.h"

#include "itkImageRegistrationMethod.h"

namespace itk
{

template< class TImage >
class ImageToImageRegistrationMethod 
: public ProcessObject
  {

  public:

    typedef ImageToImageRegistrationMethod            Self;
    typedef ProcessObject                             Superclass;
    typedef SmartPointer< Self >                      Pointer;
    typedef SmartPointer< const Self >                ConstPointer;

    itkTypeMacro( ImageToImageRegistrationMethod, ProcessObject );

    itkNewMacro( Self );

    //
    // Custom Typedefs
    //
    itkStaticConstMacro( ImageDimension, unsigned int,
                         TImage::ImageDimension );

    typedef Transform< double, 
                       itkGetStaticConstMacro( ImageDimension ),
                       itkGetStaticConstMacro( ImageDimension ) >
                                                      TransformType;

    typedef DataObjectDecorator< TransformType >      TransformOutputType;

    typedef typename DataObject::Pointer              DataObjectPointer;

    typedef TImage                                    ImageType;

    typedef SpatialObject< itkGetStaticConstMacro( ImageDimension )  >
                                                      MaskObjectType;

    //
    // Custom Methods
    //
    itkSetMacro( RegistrationNumberOfThreads, unsigned int );
    itkGetMacro( RegistrationNumberOfThreads, unsigned int );

    itkSetObjectMacro( Observer, Command );
    itkGetObjectMacro( Observer, Command );

    void SetFixedImage( const ImageType * fixedImage );
    itkGetConstObjectMacro( FixedImage, ImageType );

    void SetMovingImage( const ImageType * movingImage );
    itkGetConstObjectMacro( MovingImage, ImageType );

    itkSetMacro( UseMasks, bool );
    itkGetMacro( UseMasks, bool );

    itkSetObjectMacro( FixedImageMaskObject, MaskObjectType );
    itkGetObjectMacro( FixedImageMaskObject, MaskObjectType );

    itkSetObjectMacro( MovingImageMaskObject, MaskObjectType );
    itkGetObjectMacro( MovingImageMaskObject, MaskObjectType );

    itkSetObjectMacro( Transform, TransformType );
    itkGetObjectMacro( Transform, TransformType );

    const TransformOutputType *     GetOutput( void ) const;

    virtual DataObjectPointer       MakeOutput( unsigned int idx );

    unsigned long                   GetMTime( void ) const;

  protected:

    ImageToImageRegistrationMethod( void );
    virtual ~ImageToImageRegistrationMethod( void );

    virtual void    Initialize( void );

    void GenerateData( void );

    void PrintSelf( std::ostream & os, Indent indent ) const;

  private:

    ImageToImageRegistrationMethod( const Self & ); // Purposely not implemented
    void operator = ( const Self & );               // Purposely not implemented

    unsigned int                           m_RegistrationNumberOfThreads;

    typename TransformType::Pointer        m_Transform;

    Command::Pointer                       m_Observer;

    typename ImageType::ConstPointer       m_FixedImage;
    typename ImageType::ConstPointer       m_MovingImage;

    bool                                   m_UseMasks;
    typename MaskObjectType::Pointer       m_FixedImageMaskObject;
    typename MaskObjectType::Pointer       m_MovingImageMaskObject;

  };

} 

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageToImageRegistrationMethod.txx"
#endif


#endif 

