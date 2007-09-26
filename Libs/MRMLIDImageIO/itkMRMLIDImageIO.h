/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   MRML
  Module:    $RCSfile: itkMRMLIDImageIO.h,v $
  Date:      $Date: 2006-06-19 09:16:30 -0400 (Mon, 19 Jun 2006) $
  Version:   $Rev: 607 $

=========================================================================auto=*/

#ifndef __itkMRMLIDImageIO_h
#define __itkMRMLIDImageIO_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "itkMRMLIDIOWin32Header.h"

#include "itkImageIOBase.h"

class vtkMRMLVolumeNode;
class vtkImageData;

namespace itk
{
/** \class MRMLIDImageIO
 * \brief ImageIO object for reading and writing imaegs from a MRML scene
 *
 * MRMLIDImageIO is an ImageIO object that allows you to
 * retrieve/store an image in a MRML node using a standard ITK
 * ImageFileReader or ImageFileWriter.  THis allows a plugin to be
 * written once and compiled into a shared object module that Slicer
 * can communicate with directly or compiled into a command line
 * program that can be executed outside of Slicer.  In the former, the
 * plugin will be provided with a MRML ID for the "file" to read/write
 * and the ImageFileReader/Writer will use the MRMLIDImageIO object to
 * perform "IO" operations directly on a MRML scene.  In the latter,
 * the plugin will be provided with a filename for its inputs/outputs
 * and other ITK ImageIO objects will be employed by the
 * ImageFileReader/ImageFileWriter to read and write the data.
 *
 * The "filename" specified will look like a URI:
 *     slicer:<scene id>#<node id>                  - local slicer
 *     slicer://<hostname>/<scene id>#<node id>     - remote slicer
 *
 * This code was written on the Massachusettes Turnpike with extreme
 * glare on the LCD.
 */
class MRMLIDImageIO_EXPORT MRMLIDImageIO : public ImageIOBase
{
public:
  /** Standard class typedefs. */
  typedef MRMLIDImageIO            Self;
  typedef ImageIOBase  Superclass;
  typedef SmartPointer<Self>  Pointer;
  
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MRMLIDImageIO, ImageIOBase);

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  virtual bool CanReadFile(const char*);
  
  /** Set the spacing and dimension information for the set filename. */
  virtual void ReadImageInformation();
  
  /** Reads the data from disk into the memory buffer provided. */
  virtual void Read(void* buffer);

  /*-------- This part of the interfaces deals with writing data. ----- */

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  virtual bool CanWriteFile(const char*);

  /** Writes the header of the image.
   * Assumes SetFileName has been called with a valid file name. */
  virtual void WriteImageInformation();

  /** Writes the data to disk from the memory buffer provided. Make sure
   * that the IORegion has been set properly. */
  virtual void Write(const void* buffer);

protected:
  MRMLIDImageIO();
  ~MRMLIDImageIO();
  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Write the image information to the node and specified image */
  virtual void WriteImageInformation(vtkMRMLVolumeNode *, vtkImageData*);
  
private:
  MRMLIDImageIO(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  bool IsAVolumeNode(const char*);
  vtkMRMLVolumeNode* FileNameToVolumeNodePtr(const char*);

  //BTX
  std::string Scheme;
  std::string Authority;
  std::string SceneID;
  std::string NodeID;
  //ETX
  
};


} // end namespace itk
#endif // __itkMRMLIDImageIO_h

