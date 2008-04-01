#ifndef __itkDiffusionTensor3DFSAffineTransform_h
#define __itkDiffusionTensor3DFSAffineTransform_h


#include "itkDiffusionTensor3DAffineTransform.h"
#include <vnl/vnl_matrix.h>
#include <vnl/algo/vnl_real_eigensystem.h>
#include <vnl/vnl_real.h>



namespace itk
{
/**
 * \class DiffusionTensor3DFSAffineTransform
 * 
 * 
 * This class implements an affine transformation for diffusion tensors. It implements the 
 * Finite Strain method presented in the following paper:
 * D.C. Alexander, Member IEEE, C. Pierpaoli, P.J. Basser and J.C Gee: 
 * Spatial Transformations of Diffusion Tensor Magnetic Resonance Images, 
 * IEEE Transactions on Medical Imaging, Vol 20, No. 11, November 2001 
 * 
 * Any nonsingular matrix F can be decomposed into a rigid rotation component R, and a deformation copmoment U, where:
 * F=UR
 * 
 * The tensor position is computed by the given affine transformation F but the tensor is transformed by 
 * the extracted rotation matrix R where
 * R=(FF_{T}^{-1/2}F
 */
template<class TData>
class DiffusionTensor3DFSAffineTransform :
  public DiffusionTensor3DAffineTransform< TData >
{
public:
  typedef TData DataType ;
  typedef DiffusionTensor3DFSAffineTransform Self ;
  typedef DiffusionTensor3DAffineTransform< DataType > Superclass ;
  typedef typename Superclass::MatrixTransformType MatrixTransformType;
  typedef typename Superclass::InternalMatrixTransformType InternalMatrixTransformType ;
  typedef SmartPointer< Self > Pointer ;
  typedef SmartPointer< const Self > ConstPointer ;
  
  itkNewMacro(Self);


protected:
  void PreCompute();

private:
  MatrixTransformType ComputeMatrixSquareRoot( MatrixTransformType matrix ) ;
  MatrixTransformType ComputeRotationMatrixFromTransformationMatrix() ;

};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkDiffusionTensor3DFSAffineTransform.txx"
#endif

#endif
