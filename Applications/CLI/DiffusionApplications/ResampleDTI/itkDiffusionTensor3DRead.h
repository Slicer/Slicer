#ifndef __itkDiffusionTensor3DRead_h
#define __itkDiffusionTensor3DRead_h

#include <itkObject.h>
#include <itkMetaDataObject.h>
#include <itkOrientedImage.h>
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

template< class TData >
class DiffusionTensor3DRead : public Object
{
public:
  typedef TData DataType ;
  typedef DiffusionTensor3DRead Self ;
  typedef Matrix< double , 3 , 3 > MatrixType ;
  typedef DiffusionTensor3D< DataType > TensorDataType ;
  typedef OrientedImage< TensorDataType , 3 > DiffusionImageType ;
  typedef typename DiffusionImageType::Pointer DiffusionImagePointer ;
  typedef ImageFileReader< DiffusionImageType > FileReaderType ;
  typedef MetaDataDictionary DictionaryType ;
  typedef MetaDataObject< std::string > MetaDataStringType ;
  typedef std::vector< std::vector< double > > DoubleVectorType ;
  typedef MetaDataObject< DoubleVectorType > MetaDataDoubleVectorType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  
  itkNewMacro( Self ) ;
  int Update( const char* input ) ;
  ///Get the measurement frame of the image
  itkGetMacro( MeasurementFrame , MatrixType) ;
  ///Get the MetaDataDictionary of the image
  DictionaryType GetMetaDataDictionary() const ;
  ///Get the output of the reader, the pixels of the image being of the DiffusionTensor3D type
  DiffusionImagePointer GetOutput() const ;
  ///Set Number Of Threads
  itkSetMacro( NumberOfThreads , unsigned int);
private:
  DiffusionTensor3DRead() ;
  typename FileReaderType::Pointer m_Reader ;
  MatrixType m_MeasurementFrame ;
  unsigned int m_NumberOfThreads;
};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DRead.txx"
#endif

#endif
