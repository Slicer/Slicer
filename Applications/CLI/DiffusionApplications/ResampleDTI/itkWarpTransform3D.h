#ifndef __itkWarpTransform3D_h
#define __itkWarpTransform3D_h

#include <itkTransform.h>
#include "dtiprocessFiles/dtitypes.h"
#include <itkImage.h>
#include "itkConstNeighborhoodIterator.h"


namespace itk
{
/** \class WarpTransform3D
 * 
 * This is a class to represent warp transforms
 */
template< class FieldData >
class WarpTransform3D : public Transform< FieldData , 3 , 3 >
{
public:
  typedef FieldData FieldDataType ; 
  typedef WarpTransform3D Self ;
  typedef Transform< FieldDataType  , 3 , 3 > Superclass ;
  typedef typename Superclass::JacobianType JacobianType ;
  typedef typename Superclass::InputPointType InputPointType ;
  typedef typename Superclass::OutputPointType OutputPointType ;
  typedef DeformationImageType::Pointer DeformationImagePointerType ;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self> ConstPointer;
  typedef ConstNeighborhoodIterator<DeformationImageType> ConstNeighborhoodIteratorType;
  typedef typename ConstNeighborhoodIteratorType::RadiusType RadiusType;  

  itkNewMacro( Self ) ;
  itkTypeMacro( WarpTransform3D, Transform ) ;
  OutputPointType TransformPoint( const InputPointType & inputPoint ) const ;
  const JacobianType & GetJacobian( const InputPointType &inputPoint ) const ;
  void SetDeformationField( DeformationImagePointerType deformationField ) ;
protected:
  /** Get/Set the neighborhood radius used for gradient computation */
  itkGetConstReferenceMacro( NeighborhoodRadius, RadiusType ) ;
  itkSetMacro( NeighborhoodRadius, RadiusType ) ;
  WarpTransform3D() ;
  void operator=(const Self&); //purposely not implemented
  RadiusType m_NeighborhoodRadius ;
  double m_DerivativeWeights[ 3 ] ;
  DeformationImagePointerType m_DeformationField ;
//  Vector< double , 3 > m_OutputSpacing ;
  Size< 3 > m_SizeForJacobian ;
};

}//end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkWarpTransform3D.txx"
#endif


#endif
