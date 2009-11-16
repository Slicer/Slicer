/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFixedRotationSimilarity3DTransform.h,v $
  Language:  C++
  Date:      $Date: 2006/08/09 04:35:32 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkFixedRotationSimilarity3DTransform_h
#define __itkFixedRotationSimilarity3DTransform_h

#include <iostream>
#include "itkSimilarity3DTransform.h"

namespace itk
{

/** \brief FixedRotationSimilarity3DTransform of a vector space (e.g. space
 * coordinates)
 *
 * This transform applies a rotation, translation and scaling to the
 * space.  Only scaling and translation are available as optimized
 * parameters and rotation is treated as a fixed preset.
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
template < class TScalarType=double >    // Data type for scalars (float or double)
class ITK_EXPORT FixedRotationSimilarity3DTransform : 
      public Similarity3DTransform< TScalarType > 
{
public:
  /** Standard class typedefs. */
  typedef FixedRotationSimilarity3DTransform Self;
  typedef Similarity3DTransform< TScalarType >  Superclass;
  typedef SmartPointer<Self>                     Pointer;
  typedef SmartPointer<const Self>               ConstPointer;
    
  /** New macro for creation of through a Smart Pointer. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( FixedRotationSimilarity3DTransform, Similarity3DTransform );

  /** Dimension of parameters. */
  itkStaticConstMacro(SpaceDimension, unsigned int, 3);
  itkStaticConstMacro(InputSpaceDimension, unsigned int, 3);
  itkStaticConstMacro(OutputSpaceDimension, unsigned int, 3);
  // Parameters are only translation and uniform scale
  itkStaticConstMacro(ParametersDimension, unsigned int, 4);

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

 /** Directly set the rotation matrix of the transform.
  * \warning The input matrix must be orthogonal with isotropic scaling
  * to within a specified tolerance, else an exception is thrown.
  *
  * \sa MatrixOffsetTransformBase::SetMatrix() */
  // virtual void SetMatrix(const MatrixType &matrix);
  
  /** Set the transformation from a container of parameters This is typically
   * used by optimizers.  There are 4 parameters. The first three represent the
   * translation, the next one represents the uniform scale. */
  virtual void SetParameters( const ParametersType & parameters );
  virtual const ParametersType& GetParameters(void) const;

  virtual void SetIdentity();

  /** This method computes the Jacobian matrix of the transformation.
   * given point or vector, returning the transformed point or
   * vector. The rank of the Jacobian will also indicate if the 
   * transform is invertible at this point. */
  virtual const JacobianType & GetJacobian(const InputPointType  &point ) const;

protected:
  FixedRotationSimilarity3DTransform(unsigned int outputSpaceDim,
                         unsigned int paramDim);
  FixedRotationSimilarity3DTransform(const MatrixType & matrix,
                         const OutputVectorType & offset);
  FixedRotationSimilarity3DTransform();
  virtual ~FixedRotationSimilarity3DTransform(){};

  virtual void PrintSelf(std::ostream &os, Indent indent) const;

  // ComputeMatrix uses implementation from superclass

  // Use superclass
  //virtual void ComputeMatrixParameters();
  
private:
  FixedRotationSimilarity3DTransform(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

}; //class FixedRotationSimilarity3DTransform


}  // namespace itk

// Define instantiation macro for this template.
#define ITK_TEMPLATE_FixedRotationSimilarity3DTransform(_, EXPORT, x, y) namespace itk { \
  _(1(class EXPORT FixedRotationSimilarity3DTransform< ITK_TEMPLATE_1 x >)) \
  namespace Templates { typedef FixedRotationSimilarity3DTransform< ITK_TEMPLATE_1 x > FixedRotationSimilarity3DTransform##y; } \
  }

/*
#if ITK_TEMPLATE_EXPLICIT
# include "Templates/itkFixedRotationSimilarity3DTransform+-.h"
#endif
*/

#if ITK_TEMPLATE_TXX
# include "itkFixedRotationSimilarity3DTransform.txx"
#endif

#endif /* __itkFixedRotationSimilarity3DTransform_h */
