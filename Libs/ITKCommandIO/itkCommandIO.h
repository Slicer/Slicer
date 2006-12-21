/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   ITKCommandIO
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
#ifndef __itkCommandIO_h
#define __itkCommandIO_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkCommandIOWin32Header.h"

#include "itkImageIOBase.h"
#include <fstream>
#include "NrrdIO/NrrdIO.h"

namespace itk
{

/** \class CommandIO
 *
 * \brief Read and write the "Nearly Raw Raster Data" (Nrrd) image format.
 * The Nrrd format was developed as part of the Teem package
 * (teem.sourceforge.net).
 *
 *  \ingroup IOFilters
 */
class CommandIO_EXPORT CommandIO : public ImageIOBase
{
public:
  /** Standard class typedefs. */
  typedef CommandIO            Self;
  typedef ImageIOBase  Superclass;
  typedef SmartPointer<Self>  Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(CommandIO, ImageIOBase);

  /** The different types of ImageIO's can support data of varying
   * dimensionality. For example, some file formats are strictly 2D
   * while others can support 2D, 3D, or even n-D. This method returns
   * true/false as to whether the ImageIO can support the dimension
   * indicated. */
  virtual bool SupportsDimension(unsigned long );

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  virtual bool CanReadFile(const char*);

  /** Set the spacing and dimension information for the set filename. */
  virtual void ReadImageInformation();

  /** Reads the data from command pipe into the memory buffer provided. */
  virtual void Read(void* buffer);

  /** Determine the file type. Returns true if this ImageIO can write the
   * file specified. */
  virtual bool CanWriteFile(const char*);

  /** Set the spacing and dimension information for the set filename. */
  virtual void WriteImageInformation();

  /** Writes the data to command pipe from the memory buffer provided. Make sure
   * that the IORegions has been set properly. */
  virtual void Write(const void* buffer);

protected:
  CommandIO() {}
  ~CommandIO() {}
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Utility functions for converting between enumerated data type
      representations */
  int ITKToNrrdComponentType( const ImageIOBase::IOComponentType ) const;
  ImageIOBase::IOComponentType NrrdToITKComponentType( const int ) const;

private:
  CommandIO(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace itk

#endif // __itkCommandIO_h
