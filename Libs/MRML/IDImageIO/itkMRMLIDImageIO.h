/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef itkMRMLIDImageIO_h
#define itkMRMLIDImageIO_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include <vtkVersion.h>

#include "itkMRMLIDIOExport.h"

#include "itkImageIOBase.h"

class vtkMRMLVolumeNode;
class vtkMRMLDiffusionWeightedVolumeNode;
class vtkMRMLDiffusionImageVolumeNode;
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
 *     <code>slicer:\<scene id\>#\<node id\></code>                    - local slicer
 *     <code>slicer://\<hostname\>/\<scene id\>#\<node id\></code>     - remote slicer
 *
 * This code was written on the Massachusettes Turnpike with extreme
 * glare on the LCD.
 */
class MRMLIDImageIO_EXPORT MRMLIDImageIO : public ImageIOBase
{
public:
  /** Standard class typedefs. */
  typedef MRMLIDImageIO       Self;
  typedef ImageIOBase         Superclass;
  typedef SmartPointer<Self>  Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MRMLIDImageIO, ImageIOBase);

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  bool CanReadFile(const char*) override;

  virtual bool CanUseOwnBuffer();
  virtual void ReadUsingOwnBuffer();
  virtual void * GetOwnBuffer();

  /** Set the spacing and dimension information for the set filename. */
  void ReadImageInformation() override;

  /** Reads the data from disk into the memory buffer provided. */
  void Read(void* buffer) override;

  /*-------- This part of the interfaces deals with writing data. ----- */

  /** Determine the file type. Returns true if this ImageIO can read the
   * file specified. */
  bool CanWriteFile(const char*) override;

  /** Writes the header of the image.
   * Assumes SetFileName has been called with a valid file name. */
  void WriteImageInformation() override;

  /** Writes the data to disk from the memory buffer provided. Make sure
   * that the IORegion has been set properly. */
  void Write(const void* buffer) override;

protected:
  MRMLIDImageIO();
  ~MRMLIDImageIO() override;
  void PrintSelf(std::ostream& os, Indent indent) const override;

  /** Write the image information to the node and specified image */
  virtual void WriteImageInformation(vtkMRMLVolumeNode *, vtkImageData*,
                                     int *scalarType, int *numberOfScalarComponents);

  /** Take information in a Slicer node and transfer it the
   *  MetaDataDictionary in ITK */
  void SetDWDictionaryValues(MetaDataDictionary &dict,
                             vtkMRMLDiffusionWeightedVolumeNode *dw);

  /** Take information in a Slicer node and transfer it the
   *  MetaDataDictionary in ITK */
  void SetDTDictionaryValues(MetaDataDictionary &dict,
                             vtkMRMLDiffusionImageVolumeNode *di);

  /** Take information from the MetaDataDictionary that is needed to
   *  transfer this volume to a Slicer node */
  void SetDWNodeValues(vtkMRMLDiffusionWeightedVolumeNode *dw,
                       MetaDataDictionary &dict);

  /** Take information from the MetaDataDictionary that is needed to
   *  transfer this volume to a Slicer node */
  void SetDTNodeValues(vtkMRMLDiffusionImageVolumeNode *di,
                       MetaDataDictionary &dict);

private:
  MRMLIDImageIO(const Self&) = delete;
  void operator=(const Self&) = delete;

  bool IsAVolumeNode(const char*);
  vtkMRMLVolumeNode* FileNameToVolumeNodePtr(const char*);

  std::string m_Scheme;
  std::string m_Authority;
  std::string m_SceneID;
  std::string m_NodeID;

};


} /// end namespace itk
#endif /// itkMRMLIDImageIO_h
