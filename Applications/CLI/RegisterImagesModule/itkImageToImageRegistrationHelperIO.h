/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkitkImageToImageRegistrationHelperIO.h,v $
  Language:  C++
  Date:      $Date: 2007/03/22 14:28:52 $
  Version:   $1.0$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkImageToImageRegistrationHelperIO_h
#define __itkImageToImageRegistrationHelperIO_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkImageToImageRegistrationHelper.h"
#include "itkXMLFile.h"

namespace itk
{

/** \class ImageToImageRegistrationHelperReader
 * 
 * Reads an XML-format file containing registration parameters
 */
template< class TImage >
class ImageToImageRegistrationHelperReader 
: public XMLReader< ImageToImageRegistrationHelper< TImage > >
{
  public:

    /** Standard typedefs */ 
    typedef ImageToImageRegistrationHelperReader       Self;
    typedef XMLReader< ImageToImageRegistrationHelper< TImage>  >
                                                       Superclass;
    typedef SmartPointer< Self >                       Pointer;
  
    typedef ImageToImageRegistrationHelper< TImage >   ImageHelperType;
  
    /** Std itk macros */
    itkTypeMacro(ImageToImageRegistrationHelperReader, XMLReader);
  
    itkNewMacro(Self);
  
    /** Typedefs */
    typedef ImageToImageRegistrationHelper< TImage >   RegistrationHelperType;
  
  public:

    /** Determine if a file can be read */
    virtual int CanReadFile(const char* name);

  protected:

    virtual void StartElement(const char * name,const char **atts);
    virtual void EndElement(const char *name);

    virtual void CharacterDataHandler(const char *inData, int inLength);

  private:

    RegistrationHelperType::Pointer        m_RegistrationHelper;

    std::string                            m_CurCharacterData;

};

/** \class ImageToImageRegistrationHelperWriter
 * 
 * Writes an XML-format file containing a list of polygons,
 * based on a PolygonGroupSpatialObject.
 */
template < class TImage >
class ImageToImageRegistrationHelperWriter 
: public XMLWriterBase< ImageToImageRegistrationHelper< TImage > >
{
  public:

    /** standard typedefs */
    typedef ImageToImageRegistrationHelperWriter     Self;
    typedef XMLWriterBase< ImageToImageRegistrationHelper< TImage > > 
                                                     Superclass;
    typedef SmartPointer<Self>                       Pointer;
  
    /** Std itk macros */
    itkTypeMacro(ImageToImageRegistrationHelperWriter, XMLWriter);

    itkNewMacro(Self);
  
    /** Typedefs */
    typedef ImageToImageRegistrationHelper< TImage >   RegistrationHelperType;

    /** Test whether a file is writable. */
    virtual int CanWriteFile(const char* name);

    /** Actually write out the file in question */
    virtual int WriteFile();
};

}
#endif
