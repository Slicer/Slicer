#ifndef __itkDiffusionTensor3DExtended_h
#define __itkDiffusionTensor3DExtended_h

#include <itkDiffusionTensor3D.h>
#include <itkMatrix.h>

namespace itk
{
    
/** \class DiffusionTensor3DExtended
 * 
 * Implementation of a class that allows to transforms diffusion tensors
 *  into symmetric-matrices (to compute transformed tensors) and transform back
 * the matrices to tensors
 */
    
template< class T >
class DiffusionTensor3DExtended : public DiffusionTensor3D< T >
{
public :
  typedef T DataType;
  typedef DiffusionTensor3DExtended Self ;
  typedef DiffusionTensor3D< DataType > Superclass ;
  typedef Matrix< DataType , 3 , 3 > MatrixType ;
  DiffusionTensor3DExtended() {}
  DiffusionTensor3DExtended( const Superclass &tensor ) ;
  ///Get a Symmetric matrix representing the tensor
  MatrixType GetTensor2Matrix() ;
  ///Set the Tensor from a symmetric matrix
  void SetTensorFromMatrix( MatrixType matrix ) ;
  ///Cast the component values of the tensor
  template< class C > operator DiffusionTensor3DExtended< C > const() ;

};


}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DExtended.txx"
#endif

#endif
