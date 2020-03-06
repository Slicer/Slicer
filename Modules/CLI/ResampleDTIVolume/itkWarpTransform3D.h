#ifndef itkWarpTransform3D_h
#define itkWarpTransform3D_h

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
  typedef typename Superclass::JacobianPositionType          JacobianPositionType;
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
  static Pointer New();

  /** CreateAnother method will clone the existing instance of this type,
   * including its internal member variables. */
  ::itk::LightObject::Pointer CreateAnother() const override;

  /** Run-time type information (and related methods). */
  itkTypeMacro(WarpTransform3D, Transform);

  OutputPointType TransformPoint( const InputPointType & inputPoint ) const override;

  void ComputeJacobianWithRespectToParameters(const InputPointType  & p,
      JacobianType & jacobian ) const override;

  void ComputeJacobianWithRespectToPosition(
    const InputPointType & itkNotUsed(x),
    JacobianType & itkNotUsed(j) ) const override
  {
    itkExceptionMacro("ComputeJacobianWithRespectToPosition is not implemented for WarpTransform3D");
  }

  void ComputeJacobianWithRespectToPosition(
    const InputPointType & itkNotUsed(x),
    JacobianPositionType & itkNotUsed(j) ) const override
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
  OutputVectorType    TransformVector(const InputVectorType &) const override
  {
    itkExceptionMacro("TransformVector(const InputVectorType &) is not implemented for WarpTransform3D");
  }

  /**  Method to transform a vnl_vector. */
  OutputVnlVectorType TransformVector(const InputVnlVectorType &) const override
  {
    itkExceptionMacro("TransformVector(const InputVnlVectorType &) is not implemented for WarpTransform3D");
  }

  using Superclass::TransformCovariantVector;
  /**  Method to transform a CovariantVector. */
  OutputCovariantVectorType TransformCovariantVector(const InputCovariantVectorType &) const  override
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
  void  SetParameters(const ParametersType &) override
  {
  }
  /**This is a dummy function. This class does not allow to set the
   * transform fixed parameters through this function. Use
   * SetDeformationField() to set the transform */
  void  SetFixedParameters(const ParametersType &) override
  {
  }

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
