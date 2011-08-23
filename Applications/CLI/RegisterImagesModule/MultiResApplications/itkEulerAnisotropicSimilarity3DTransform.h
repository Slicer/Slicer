/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEulerAnisotropicSimilarity3DTransform.h,v $
  Language:  C++
  Date:      $Date: 2009-03-03 15:09:08 $
  Version:   $Revision: 1.14 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkEulerAnisotropicSimilarity3DTransform_h
#define __itkEulerAnisotropicSimilarity3DTransform_h

#include <iostream>
#include "itkEuler3DTransform.h"

namespace itk
{

/** \class EulerAnisotropicSimilarity3DTransform
 * \brief EulerAnisotropicSimilarity3DTransform of a vector space (e.g. space coordinates)
 *
 * This transform applies a versor rotation and translation & scale/skew
 * to the space
 *
 * The parameters for this transform can be set either using individual Set
 * methods or in serialized form using SetParameters() and SetFixedParameters().
 *
 * The serialization of the optimizable parameters is an array of 25 elements.
 * The first 3 elements are the euler angles for the
 * 3D rotation. The next 3 parameters defines the translation in each
 * dimension. The next 3 parameters defines scaling in each dimension.
 * The last 3 parameters defines the skew.
 *
 * The serialization of the fixed parameters is an array of 3 elements defining
 * the center of rotation.
 *
 *
 * \ingroup Transforms
 */
template < class TScalarType=double >  // Data type for scalars:float or double
class ITK_EXPORT EulerAnisotropicSimilarity3DTransform : 
            public Euler3DTransform< TScalarType > 
{
public:
  /** Standard class typedefs. */
  typedef EulerAnisotropicSimilarity3DTransform             Self;
  typedef Euler3DTransform< TScalarType >         Superclass;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
      
  /** New macro for creation of through a Smart Pointer. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( EulerAnisotropicSimilarity3DTransform, Euler3DTransform );

  /** Dimension of parameters. */
  itkStaticConstMacro(InputSpaceDimension, unsigned int, 3);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, 3);
  itkStaticConstMacro(ParametersDimension, unsigned int, 9);

  /** Parameters Type   */
  typedef typename Superclass::ParametersType         ParametersType;
  typedef typename Superclass::JacobianType           JacobianType;
  typedef typename Superclass::ScalarType             ScalarType;
  typedef typename Superclass::InputPointType         InputPointType;
  typedef typename Superclass::OutputPointType        OutputPointType;
  typedef typename Superclass::InputVectorType        InputVectorType;
  typedef typename Superclass::OutputVectorType       OutputVectorType;
  typedef typename Superclass::InputVnlVectorType     InputVnlVectorType;
  typedef typename Superclass::OutputVnlVectorType    OutputVnlVectorType;
  typedef typename Superclass::InputCovariantVectorType 
                                                      InputCovariantVectorType;
  typedef typename Superclass::OutputCovariantVectorType
                                                      OutputCovariantVectorType;
  typedef typename Superclass::MatrixType             MatrixType;
  typedef typename Superclass::InverseMatrixType      InverseMatrixType;
  typedef typename Superclass::CenterType             CenterType;
  typedef typename Superclass::OffsetType             OffsetType;
  typedef typename Superclass::TranslationType        TranslationType;

  typedef typename Superclass::AngleType              AngleType;

  /** Scale & Skew Vector Type. */
  typedef Vector<TScalarType, 3> 
                                                      ScaleVectorType;

 /** Directly set the matrix of the transform.
  *
  * \sa MatrixOffsetTransformBase::SetMatrix() */
  virtual void SetMatrix(const MatrixType &matrix);

  /** Set the transformation from a container of parameters
   * This is typically used by optimizers.
   * There are 9 parameters:
   *   0-2   euler angles
   *   3-5   translation
   *   6-8   Scale
   **  */
  virtual void SetParameters( const ParametersType & parameters );
  virtual const ParametersType& GetParameters(void) const;

  void SetScale( const ScaleVectorType & scale );
  itkGetConstReferenceMacro( Scale, ScaleVectorType );

  void SetIdentity();

  /** This method computes the Jacobian matrix of the transformation.
   * given point or vector, returning the transformed point or
   * vector. The rank of the Jacobian will also indicate if the 
   * transform is invertible at this point. */
  virtual const JacobianType & GetJacobian(const InputPointType  &point ) const;
  virtual void ComputeJacobianWithRespectToParameters(const InputPointType  &point, JacobianType & jacobian) const;
protected:
  EulerAnisotropicSimilarity3DTransform();
  EulerAnisotropicSimilarity3DTransform(const MatrixType &matrix,
                             const OutputVectorType &offset);
  EulerAnisotropicSimilarity3DTransform(unsigned int paramDims);
  ~EulerAnisotropicSimilarity3DTransform(){}

  void PrintSelf(std::ostream &os, Indent indent) const;

  void SetVarScale(const ScaleVectorType & scale)
    { m_Scale = scale; }

  /** Compute the components of the rotation matrix in the superclass. */
  void ComputeMatrix(void);
  void ComputeMatrixParameters(void);

private:
  EulerAnisotropicSimilarity3DTransform(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /**  Vector containing the scale. */
  ScaleVectorType          m_Scale;
  mutable JacobianType     m_NonThreadsafeSharedJacobian;

}; //class EulerAnisotropicSimilarity3DTransform


}  // namespace itk

// Define instantiation macro for this template.
#define ITK_TEMPLATE_EulerAnisotropicSimilarity3DTransform(_, EXPORT, x, y) namespace itk { \
  _(1(class EXPORT EulerAnisotropicSimilarity3DTransform< ITK_TEMPLATE_1 x >)) \
  namespace Templates { typedef EulerAnisotropicSimilarity3DTransform< ITK_TEMPLATE_1 x > \
                                                  EulerAnisotropicSimilarity3DTransform##y; } \
  }

#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkEulerAnisotropicSimilarity3DTransform+-.h"
#endif

#if ITK_TEMPLATE_TXX
# include "itkEulerAnisotropicSimilarity3DTransform.txx"
#endif


#endif /* __EulerAnisotropicSimilarity3DTransform_h */
