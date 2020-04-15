/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkAnisotropicSimilarity3DTransform.h,v $
  Language:  C++
  Date:      $Date: 2006/08/09 04:35:32 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkAnisotropicSimilarity3DTransform_h
#define itkAnisotropicSimilarity3DTransform_h

#include <iostream>
#include "itkVersorRigid3DTransform.h"

namespace itk
{

/** \class AnisotropicSimilarity3DTransform
 * \brief AnisotropicSimilarity3DTransform of a vector space (e.g. space
 * coordinates)
 *
 * This transform applies a rotation, translation and anisotropic scaling
 * to the space.
 *
 * The parameters for this transform can be set either using individual
 * Set methods or in serialized form using SetParameters() and
 * SetFixedParameters().
 *
 * The serialization of the optimizable parameters is an array of 9
 * elements. The first 3 elements are the components of the versor
 * representation of 3D rotation. The next 3 parameters defines the
 * translation in each dimension. The last parameter defines the
 * anisotropic scaling.
 *
 * The serialization of the fixed parameters is an array of 3 elements
 * defining the center of rotation.
 *
 * \ingroup Transforms
 *
 * \sa VersorRigid3DTransform
 */
template <class TScalarType = double>
// Data type for scalars (float or double)
class AnisotropicSimilarity3DTransform :
  public VersorRigid3DTransform<TScalarType>
{
public:
  /** Standard class typedefs. */
  using Self = AnisotropicSimilarity3DTransform<TScalarType>;
  using Superclass = VersorRigid3DTransform<TScalarType>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** New macro for creation of through a Smart Pointer. */
  itkNewMacro( Self );

  /** Explicitly deleted functions belong in the public interface */
  AnisotropicSimilarity3DTransform(const Self &) = delete;
  void operator=(const Self &) = delete;

  /** Run-time type information (and related methods). */
  itkTypeMacro( AnisotropicSimilarity3DTransform, VersorRigid3DTransform );

  /** Dimension of parameters. */
  itkStaticConstMacro(SpaceDimension, unsigned int, 3);
  itkStaticConstMacro(InputSpaceDimension, unsigned int, 3);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, 3);
  itkStaticConstMacro(ParametersDimension, unsigned int, 9);

  /** Parameters Type   */
  using ParametersType = typename Superclass::ParametersType;
  using JacobianType = typename Superclass::JacobianType;
  using ScalarType = typename Superclass::ScalarType;
  using InputPointType = typename Superclass::InputPointType;
  using OutputPointType = typename Superclass::OutputPointType;
  using InputVectorType = typename Superclass::InputVectorType;
  using OutputVectorType = typename Superclass::OutputVectorType;
  using InputVnlVectorType = typename Superclass::InputVnlVectorType;
  using OutputVnlVectorType = typename Superclass::OutputVnlVectorType;

  using InputCovariantVectorType = typename Superclass::InputCovariantVectorType;
  using OutputCovariantVectorType = typename Superclass::OutputCovariantVectorType;

  using MatrixType = typename Superclass::MatrixType;
  using InverseMatrixType = typename Superclass::InverseMatrixType;
  using CenterType = typename Superclass::CenterType;
  using OffsetType = typename Superclass::OffsetType;
  using TranslationType = typename Superclass::TranslationType;

  /** Versor type. */
  using VersorType = typename Superclass::VersorType;
  using AxisType = typename Superclass::AxisType;
  using AngleType = typename Superclass::AngleType;
  using VectorType = typename Superclass::InputVectorType;
  using ScaleType = TScalarType;

  /** Directly set the rotation matrix of the transform.
   * \warning The input matrix must be orthogonal with isotropic scaling
   * to within a specified tolerance, else an exception is thrown.
   *
   * \sa MatrixOffsetTransformBase::SetMatrix() */
  using itk::Rigid3DTransform<TScalarType>::SetMatrix;
  void SetMatrix(const MatrixType & matrix) override;

  /** Set the transformation from a container of parameters This is typically
   * used by optimizers.  There are 7 parameters. The first three represent the
   * versor, the next three represent the translation and the last one
   * represents the scaling factor. */
  void SetParameters( const ParametersType & parameters ) override;

  const ParametersType& GetParameters() const override;

  /** Set/Get the value of the isotropic scaling factor */
  void SetScale( ScaleType scale );

  void SetScale( VectorType scale );

  itkGetConstReferenceMacro( Scale, VectorType );

  /** This method computes the Jacobian matrix of the transformation.
   * given point or vector, returning the transformed point or
   * vector. The rank of the Jacobian will also indicate if the
   * transform is invertible at this point. */
  void ComputeJacobianWithRespectToParameters(const InputPointType & p, JacobianType & jacobian) const override;

protected:
  AnisotropicSimilarity3DTransform(const MatrixType & matrix, const OutputVectorType & offset);
  AnisotropicSimilarity3DTransform(unsigned int paramDim);
  AnisotropicSimilarity3DTransform();
  ~AnisotropicSimilarity3DTransform() override = default;;

  void PrintSelf(std::ostream & os, Indent indent) const override;

  /** Recomputes the matrix by calling the Superclass::ComputeMatrix() and then
   * applying the scale factor. */
  void ComputeMatrix() override;

  /** Computes the parameters from an input matrix. */
  void ComputeMatrixParameters() override;

private:

  VectorType           m_Scale;

}; // class AnisotropicSimilarity3DTransform

}  // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAnisotropicSimilarity3DTransform.txx"
#endif

#endif /* itkAnisotropicSimilarity3DTransform_h */
