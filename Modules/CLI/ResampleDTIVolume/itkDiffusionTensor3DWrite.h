/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DWrite_h
#define itkDiffusionTensor3DWrite_h

#include <itkObject.h>
#include <itkMetaDataObject.h>
#include <itkImage.h>
#include <itkMatrix.h>
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include <itkImageIOBase.h>
#include "itkDiffusionTensor3D.h"

namespace itk
{

/** \class DiffusionTensor3DWrite
 *
 * Write diffusion tensor image files
 */
template <class TData>
class DiffusionTensor3DWrite : public Object
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DWrite<TData>;
  using TensorDataType = DiffusionTensor3D<DataType>;
  using DiffusionImageType = Image<TensorDataType, 3>;
  using DictionaryType = MetaDataDictionary;
  using WriterType = ImageFileWriter<DiffusionImageType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;
  using DoubleVectorType = std::vector<std::vector<double> >;
  using MetaDataDoubleVectorType = MetaDataObject<DoubleVectorType>;
  using MetaDataIntType = MetaDataObject<std::string>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DWrite, Object);

  itkNewMacro( Self );
  // /Set input tensor image
  itkSetObjectMacro( Input, DiffusionImageType );
  // /Write the image in the given file
  int Update( const char* output );

  // /Set the metadatadictionary of the image, including its measurement frame
  void SetMetaDataDictionary( DictionaryType dic );

  // /Set Number of Threads
  itkSetMacro( NumberOfThreads, unsigned int);
  /**Set the Measurement frame of the image. If the measurement frame has been modified from an original image,
  * one can use SetMetaDataDictionary to copy the metadatadictionary from the original image and then
  * use this function to set the new metadatadictionary. Using these functions the other way around would not give
  * a good result.
  */
  void SetMeasurementFrame( Matrix<double, 3, 3> measurementFrame );

//  Space:
//  nrrdSpaceUnknown,
//  nrrdSpaceRightAnteriorSuperior,     /*  1: NIFTI-1 (right-handed) */
//  nrrdSpaceLeftAnteriorSuperior,      /*  2: standard Analyze (left-handed) */
//  nrrdSpaceLeftPosteriorSuperior,     /*  3: DICOM 3.0 (right-handed) */
//  nrrdSpaceRightAnteriorSuperiorTime, /*  4: */
//  nrrdSpaceLeftAnteriorSuperiorTime,  /*  5: */
//  nrrdSpaceLeftPosteriorSuperiorTime, /*  6: */
//  nrrdSpaceScannerXYZ,                /*  7: ACR/NEMA 2.0 (pre-DICOM 3.0) */
//  nrrdSpaceScannerXYZTime,            /*  8: */
//  nrrdSpace3DRightHanded,             /*  9: */
//  nrrdSpace3DLeftHanded,              /* 10: */
//  nrrdSpace3DRightHandedTime,         /* 11: */
//  nrrdSpace3DLeftHandedTime,          /* 12: */
//  nrrdSpaceLast
  void SetSpace(int space);

private:
  DiffusionTensor3DWrite();

  typename DiffusionImageType::Pointer m_Input;

  unsigned int   m_NumberOfThreads;
  DictionaryType m_MetaDataDictionary;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DWrite.txx"
#endif

#endif
