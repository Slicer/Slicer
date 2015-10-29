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
template <class FieldData>
class WarpTransform3D : public Transform<FieldData, 3, 3>
{
public:
  typedef FieldData                                          FieldDataType;
  typedef WarpTransform3D                                    Self;
  typedef Transform<FieldDataType, 3, 3>                     Superclass;
  typedef typename Superclass::JacobianType                  JacobianType;
  typedef typename Superclass::InputPointType                InputPointType;
  typedef typename Superclass::InputVectorType               InputVectorType;
  typedef typename Superclass::InputVnlVectorType            InputVnlVectorType;
  typedef typename Superclass::InputCovariantVectorType      InputCovariantVectorType;
  typedef typename Superclass::OutputPointType               OutputPointType;
  typedef typename Superclass::OutputVectorType              OutputVectorType;
  typedef typename Superclass::OutputVnlVectorType           OutputVnlVectorType;
  typedef typename Superclass::OutputCovariantVectorType     OutputCovariantVectorType;
  typedef DeformationImageType::Pointer                      DeformationImagePointerType;
  typedef SmartPointer<Self>                                 Pointer;
  typedef SmartPointer<const Self>                           ConstPointer;
  typedef ConstNeighborhoodIterator<DeformationImageType>    ConstNeighborhoodIteratorType;
  typedef typename ConstNeighborhoodIteratorType::RadiusType RadiusType;
  typedef typename Superclass::ParametersType                ParametersType;

  /** New method for creation through the object factory.
   * NOTE: itkNewMacro is not used because we need to provide a
   * CreateAnother method for this class */
  static Pointer New(void);

  /** CreateAnother method will clone the existing instance of this type,
   * including its internal member variables. */
  virtual::itk::LightObject::Pointer CreateAnother(void) const ITK_OVERRIDE;

  itkTypeMacro( WarpTransform3D, Transform );
  OutputPointType TransformPoint( const InputPointType & inputPoint ) const ITK_OVERRIDE;

  virtual void ComputeJacobianWithRespectToParameters(const InputPointType  & p, JacobianType & jacobian ) const ITK_OVERRIDE;

  virtual void ComputeJacobianWithRespectToPosition(
    const InputPointType & itkNotUsed(x),
    JacobianType & itkNotUsed(j) ) const ITK_OVERRIDE
  {
    itkExceptionMacro("ComputeJacobianWithRespectToPosition is not implemented for WarpTransform3D");
  }

  void SetDeformationField( DeformationImagePointerType deformationField );

  const DeformationImagePointerType GetDeformationField() const
  {
    return m_DeformationField;
  }

  using Superclass::TransformVector;
  /**  Method to transform a vector. */
  virtual OutputVectorType    TransformVector(const InputVectorType &) const ITK_OVERRIDE
  {
    itkExceptionMacro("TransformVector(const InputVectorType &) is not implemented for WarpTransform3D");
  }

  /**  Method to transform a vnl_vector. */
  virtual OutputVnlVectorType TransformVector(const InputVnlVectorType &) const ITK_OVERRIDE
  {
    itkExceptionMacro("TransformVector(const InputVnlVectorType &) is not implemented for WarpTransform3D");
  }

  using Superclass::TransformCovariantVector;
  /**  Method to transform a CovariantVector. */
  virtual OutputCovariantVectorType TransformCovariantVector(const InputCovariantVectorType &) const  ITK_OVERRIDE
  {
    itkExceptionMacro(
      "TransformCovariantVector(const InputCovariantVectorType & is not implemented for WarpTransform3D");
  }

protected:
  /** Get/Set the neighborhood radius used for gradient computation */
  itkGetConstReferenceMacro( NeighborhoodRadius, RadiusType );
  itkSetMacro( NeighborhoodRadius, RadiusType );

  /**This is a dummy function. This class does not allow to set the
   * transform parameters through this function. Use
   * SetDeformationField() to set the transform.*/
  virtual void  SetParameters(const ParametersType &) ITK_OVERRIDE
  {
  };
  /**This is a dummy function. This class does not allow to set the
   * transform fixed parameters through this function. Use
   * SetDeformationField() to set the transform */
  virtual void  SetFixedParameters(const ParametersType &) ITK_OVERRIDE
  {
  };

  WarpTransform3D();
  void operator=(const Self &); // purposely not implemented

  RadiusType                  m_NeighborhoodRadius;
  double                      m_DerivativeWeights[3];
  DeformationImagePointerType m_DeformationField;
//  Vector< double , 3 > m_OutputSpacing ;
  Size<3>              m_SizeForJacobian;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkWarpTransform3D.txx"
#endif

#endif
