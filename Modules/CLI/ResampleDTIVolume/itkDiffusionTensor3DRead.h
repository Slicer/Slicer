/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DRead_h
#define itkDiffusionTensor3DRead_h

#include <itkObject.h>
#include <itkMetaDataObject.h>
#include <itkImage.h>
#include <itkMatrix.h>
#include <itkImageFileReader.h>
#include <itkNrrdImageIO.h>
#include <itkImageIOBase.h>
#include <itkDiffusionTensor3D.h>

namespace itk
{

/** \class DiffusionTensor3DRead
 *
 * Read diffusion tensor image files
 */

template <class TData>
class ITK_ABI_EXPORT DiffusionTensor3DRead : public Object
{
public:
  using DataType = TData;
  using Self = DiffusionTensor3DRead<TData>;
  using MatrixType = Matrix<double, 3, 3>;
  using TensorDataType = DiffusionTensor3D<DataType>;
  using DiffusionImageType = Image<TensorDataType, 3>;
  using DiffusionImagePointer = typename DiffusionImageType::Pointer;
  using FileReaderType = ImageFileReader<DiffusionImageType>;
  using DictionaryType = MetaDataDictionary;
  using MetaDataStringType = MetaDataObject<std::string>;
  using DoubleVectorType = std::vector<std::vector<double> >;
  using MetaDataDoubleVectorType = MetaDataObject<DoubleVectorType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(DiffusionTensor3DRead, Object);

  itkNewMacro( Self );
  int Update( const char* input );

  // /Get the measurement frame of the image
  itkGetMacro( MeasurementFrame, MatrixType);
  // /Get the MetaDataDictionary of the image
  DictionaryType GetMetaDataDictionary() const;

  // /Get the output of the reader, the pixels of the image being of the DiffusionTensor3D type
  DiffusionImagePointer GetOutput() const;

  // /Set Number Of Threads
  itkSetMacro( NumberOfThreads, unsigned int);
  itkGetMacro( HasMeasurementFrame , bool );
private:
  DiffusionTensor3DRead();

  typename FileReaderType::Pointer m_Reader;

  MatrixType   m_MeasurementFrame;
  unsigned int m_NumberOfThreads;
  bool         m_HasMeasurementFrame;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DRead.txx"
#endif

#endif
