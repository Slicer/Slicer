#ifndef __itkDiffusionTensor3DMatrix3x3Transform_h
#define __itkDiffusionTensor3DMatrix3x3Transform_h


#include "itkDiffusionTensor3DTransform.h"
#include <itkSemaphore.h>
#include <itkVector.h>

namespace itk
{

/**
 * \class DiffusionTensor3DMatrix3x3Transform
 * 
 * Virtual class to implement diffusion images transformation based on a 3x3 matrix
 */

template< class TData >
class DiffusionTensor3DMatrix3x3Transform :
  public DiffusionTensor3DTransform< TData >
{
public:
  typedef TData DataType ; 
  typedef DiffusionTensor3DMatrix3x3Transform Self ;
  typedef DiffusionTensor3DTransform< DataType > Superclass ;
  typedef typename Superclass::TensorDataType TensorDataType ;
  typedef typename Superclass::InternalTensorDataType InternalTensorDataType ;
  typedef typename Superclass::PointType PointType ;
  typedef typename Superclass::MatrixTransformType MatrixTransformType ;
  typedef typename Superclass::MatrixDataType MatrixDataType ;
  typedef typename Superclass::InternalMatrixTransformType InternalMatrixTransformType ;
  typedef typename Superclass::InternalMatrixDataType InternalMatrixDataType ;
  typedef Vector< double , 3 > VectorType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;

  ///Set the translation vector
  void SetTranslation( VectorType translation ) ;
  ///Set the center of the transformation
  void SetCenter( PointType center ) ;
  ///Evaluate the position of the transformed tensor in the output image
  PointType EvaluateTensorPosition( const PointType &point ) ;
  ///Set the 3x3 transform matrix
  virtual void SetMatrix3x3( MatrixTransformType &matrix ) ;
  ///Evaluate the transformed tensor
  virtual TensorDataType EvaluateTransformedTensor( TensorDataType &tensor ) ;
  virtual TensorDataType EvaluateTransformedTensor( TensorDataType &tensor , PointType &outputPosition ) ; //dummy output position; to be compatible with non-rigid transforms
protected:
  void ComputeOffset() ;
  virtual void PreCompute() = 0 ;
  DiffusionTensor3DMatrix3x3Transform() ;
  InternalMatrixTransformType m_TransformMatrix ;
  InternalMatrixTransformType m_Transform ;
  InternalMatrixTransformType m_TransformT ;
  unsigned long latestTime ;
  VectorType m_Translation ;
  VectorType m_Offset ;
  Semaphore::Pointer P ;
  PointType m_Center ;

};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DMatrix3x3Transform.txx"
#endif

#endif
