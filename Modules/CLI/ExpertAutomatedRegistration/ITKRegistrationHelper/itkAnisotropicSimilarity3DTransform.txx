/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkAnisotropicSimilarity3DTransform.txx,v $
  Language:  C++
  Date:      $Date: 2007/11/27 16:04:48 $
  Version:   $Revision: 1.9 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef itkAnisotropicSimilarity3DTransform_txx
#define itkAnisotropicSimilarity3DTransform_txx

#include "itkAnisotropicSimilarity3DTransform.h"
#include "vnl/vnl_math.h"
#include "vnl/vnl_det.h"

namespace itk
{

// Constructor with default arguments
template <class TScalarType>
AnisotropicSimilarity3DTransform<TScalarType>
::AnisotropicSimilarity3DTransform() :
  Superclass(ParametersDimension)
{
  m_Scale[0] = 1.0;
  m_Scale[1] = 1.0;
  m_Scale[2] = 1.0;
}

// Constructor with arguments
template <class TScalarType>
AnisotropicSimilarity3DTransform<TScalarType>::AnisotropicSimilarity3DTransform(unsigned int paramDim) :
  Superclass(paramDim)
{
}

// Constructor with arguments
template <class TScalarType>
AnisotropicSimilarity3DTransform<TScalarType>::AnisotropicSimilarity3DTransform( const MatrixType & matrix,
                                                                                 const OutputVectorType & offset) :
  Superclass(matrix, offset)
{
}

// Set the scale factor
template <class TScalarType>
void
AnisotropicSimilarity3DTransform<TScalarType>
::SetScale( ScaleType scale )
{
  m_Scale[0] = scale;
  m_Scale[1] = scale;
  m_Scale[2] = scale;
  this->ComputeMatrix();
}

// Set the scale factor
template <class TScalarType>
void
AnisotropicSimilarity3DTransform<TScalarType>
::SetScale( VectorType scale )
{
  m_Scale[0] = scale[0];
  m_Scale[1] = scale[1];
  m_Scale[2] = scale[2];
  this->ComputeMatrix();
}

// Directly set the matrix
template <class TScalarType>
void
AnisotropicSimilarity3DTransform<TScalarType>
::SetMatrix( const MatrixType & matrix )
{
  //
  // Since the matrix should be an orthogonal matrix
  // multiplied by the scale factor, then its determinant
  // must be equal to the cube of the scale factor.
  //
  double det = vnl_det( matrix.GetVnlMatrix() );

  if( det == 0.0 )
    {
    itkExceptionMacro( << "Attempting to set a matrix with a zero determinant" );
    }

  double scale[3];
  scale[0] = sqrt( fabs( matrix.GetVnlMatrix()[0][0] * matrix.GetVnlMatrix()[0][0]
                         + matrix.GetVnlMatrix()[0][1] * matrix.GetVnlMatrix()[0][1]
                         + matrix.GetVnlMatrix()[0][2] * matrix.GetVnlMatrix()[0][2] ) );
  scale[1] = sqrt( fabs( matrix.GetVnlMatrix()[1][0] * matrix.GetVnlMatrix()[1][0]
                         + matrix.GetVnlMatrix()[1][1] * matrix.GetVnlMatrix()[1][1]
                         + matrix.GetVnlMatrix()[1][2] * matrix.GetVnlMatrix()[1][2] ) );
  scale[2] = sqrt( fabs( matrix.GetVnlMatrix()[2][0] * matrix.GetVnlMatrix()[2][0]
                         + matrix.GetVnlMatrix()[2][1] * matrix.GetVnlMatrix()[2][1]
                         + matrix.GetVnlMatrix()[2][2] * matrix.GetVnlMatrix()[2][2] ) );

  MatrixType scaleMatrix;
  scaleMatrix(0, 0) = scale[0];
  scaleMatrix(1, 1) = scale[1];
  scaleMatrix(2, 2) = scale[2];
  scaleMatrix = scaleMatrix.GetInverse();

  MatrixType testForOrthogonal = matrix * scaleMatrix;

  const double tolerance = 1e-10;
  if( !this->MatrixIsOrthogonal( testForOrthogonal, tolerance ) )
    {
    itkExceptionMacro( << "Attempting to set a non-orthogonal matrix (after removing scaling)" );
    }

  typedef MatrixOffsetTransformBase<TScalarType, 3> Baseclass;
  this->Baseclass::SetMatrix( matrix );
}

// Set Parameters
template <class TScalarType>
void
AnisotropicSimilarity3DTransform<TScalarType>
::SetParameters( const ParametersType & parameters )
{

  itkDebugMacro( << "Setting parameters " << parameters );

  // Transfer the versor part

  AxisType axis;

  double norm = parameters[0] * parameters[0];
  axis[0] = parameters[0];
  norm += parameters[1] * parameters[1];
  axis[1] = parameters[1];
  norm += parameters[2] * parameters[2];
  axis[2] = parameters[2];
  if( norm > 0 )
    {
    norm = std::sqrt(norm);
    }

  double epsilon = 1e-10;
  if( norm >= 1.0 - epsilon )
    {
    axis = axis / (norm + epsilon * norm);
    }
  VersorType newVersor;
  newVersor.Set(axis);
  this->SetVarVersor( newVersor );
  m_Scale[0] = parameters[6]; // must be set before calling ComputeMatrix();
  m_Scale[1] = parameters[7]; // must be set before calling ComputeMatrix();
  m_Scale[2] = parameters[8]; // must be set before calling ComputeMatrix();
  this->ComputeMatrix();

  itkDebugMacro( << "Versor is now " << this->GetVersor() );

  // Transfer the translation part
  TranslationType newTranslation;
  newTranslation[0] = parameters[3];
  newTranslation[1] = parameters[4];
  newTranslation[2] = parameters[5];
  this->SetVarTranslation(newTranslation);
  this->ComputeOffset();

  // Modified is always called since we just have a pointer to the
  // parameters and cannot know if the parameters have changed.
  this->Modified();

  itkDebugMacro(<< "After setting parameters ");
}

//
// Get Parameters
//
// Parameters are ordered as:
//
// p[0:2] = right part of the versor (axis times std::sin(t/2))
// p[3:5} = translation components
// p[6:8} = scaling factor (isotropic)
//

template <class TScalarType>
const typename AnisotropicSimilarity3DTransform<TScalarType>::ParametersType&
AnisotropicSimilarity3DTransform<TScalarType>::GetParameters() const
{
  itkDebugMacro( << "Getting parameters ");

  this->m_Parameters[0] = this->GetVersor().GetX();
  this->m_Parameters[1] = this->GetVersor().GetY();
  this->m_Parameters[2] = this->GetVersor().GetZ();

  // Transfer the translation
  this->m_Parameters[3] = this->GetTranslation()[0];
  this->m_Parameters[4] = this->GetTranslation()[1];
  this->m_Parameters[5] = this->GetTranslation()[2];

  this->m_Parameters[6] = this->GetScale()[0];
  this->m_Parameters[7] = this->GetScale()[1];
  this->m_Parameters[8] = this->GetScale()[2];

  itkDebugMacro(<< "After getting parameters " << this->m_Parameters );

  return this->m_Parameters;
}

// Set parameters
template <class TScalarType>
void
AnisotropicSimilarity3DTransform<TScalarType>::ComputeJacobianWithRespectToParameters(const InputPointType & p,
                                                                                      JacobianType & jacobian) const
{
  typedef typename VersorType::ValueType ValueType;

  // compute derivatives with respect to rotation
  const ValueType vx = this->GetVersor().GetX();
  const ValueType vy = this->GetVersor().GetY();
  const ValueType vz = this->GetVersor().GetZ();
  const ValueType vw = this->GetVersor().GetW();

  jacobian.SetSize( 3, 9 );
  jacobian.Fill(0.0);

  const InputVectorType pp = p - this->GetCenter();

  const double px = pp[0];
  const double py = pp[1];
  const double pz = pp[2];

  const double vxx = vx * vx;
  const double vyy = vy * vy;
  const double vzz = vz * vz;
  const double vww = vw * vw;

  const double vxy = vx * vy;
  const double vxz = vx * vz;
  const double vxw = vx * vw;

  const double vyz = vy * vz;
  const double vyw = vy * vw;

  const double vzw = vz * vw;

  // compute Jacobian with respect to quaternion parameters
  jacobian[0][0] = 2.0 * (               (vyw + vxz) * py + (vzw - vxy) * pz)
    / vw;
  jacobian[1][0] = 2.0 * ( (vyw - vxz) * px   - 2 * vxw   * py + (vxx - vww) * pz)
    / vw;
  jacobian[2][0] = 2.0 * ( (vzw + vxy) * px + (vww - vxx) * py   - 2 * vxw   * pz)
    / vw;

  jacobian[0][1] = 2.0 * ( -2 * vyw  * px + (vxw + vyz) * py + (vww - vyy) * pz)
    / vw;
  jacobian[1][1] = 2.0 * ( (vxw - vyz) * px                + (vzw + vxy) * pz)
    / vw;
  jacobian[2][1] = 2.0 * ( (vyy - vww) * px + (vzw - vxy) * py   - 2 * vyw   * pz)
    / vw;

  jacobian[0][2] = 2.0 * ( -2 * vzw  * px + (vzz - vww) * py + (vxw - vyz) * pz)
    / vw;
  jacobian[1][2] = 2.0 * ( (vww - vzz) * px   - 2 * vzw   * py + (vyw + vxz) * pz)
    / vw;
  jacobian[2][2] = 2.0 * ( (vxw + vyz) * px + (vyw - vxz) * py               )
    / vw;

  // compute Jacobian with respect to the translation parameters
  jacobian[0][3] = 1.0;
  jacobian[1][4] = 1.0;
  jacobian[2][5] = 1.0;

  // compute Jacobian with respect to the scale parameter
  const MatrixType & matrix = this->GetMatrix();

  const InputVectorType mpp = matrix * pp;

  jacobian[0][6] = mpp[0] / m_Scale[0];
  jacobian[1][6] = mpp[1] / m_Scale[1];
  jacobian[2][6] = mpp[2] / m_Scale[2];
  jacobian[0][7] = mpp[0] / m_Scale[0];
  jacobian[1][7] = mpp[1] / m_Scale[1];
  jacobian[2][7] = mpp[2] / m_Scale[2];
  jacobian[0][8] = mpp[0] / m_Scale[0];
  jacobian[1][8] = mpp[1] / m_Scale[1];
  jacobian[2][8] = mpp[2] / m_Scale[2];
}

// Set the scale factor
template <class TScalarType>
void
AnisotropicSimilarity3DTransform<TScalarType>
::ComputeMatrix()
{
  this->Superclass::ComputeMatrix();

  MatrixType newMatrix = this->GetMatrix();

  MatrixType scaleMatrix;
  scaleMatrix.Fill(0);
  scaleMatrix(0, 0) = m_Scale[0];
  scaleMatrix(1, 1) = m_Scale[1];
  scaleMatrix(2, 2) = m_Scale[2];
  newMatrix = newMatrix * scaleMatrix;

  this->SetVarMatrix(newMatrix);
}

/** Compute the matrix */
template <class TScalarType>
void AnisotropicSimilarity3DTransform<TScalarType>::ComputeMatrixParameters()
{
  MatrixType matrix = this->GetMatrix();

  m_Scale[0] = sqrt( fabs( matrix.GetVnlMatrix()[0][0] * matrix.GetVnlMatrix()[0][0]
                           + matrix.GetVnlMatrix()[0][1] * matrix.GetVnlMatrix()[0][1]
                           + matrix.GetVnlMatrix()[0][2] * matrix.GetVnlMatrix()[0][2] ) );
  m_Scale[1] = sqrt( fabs( matrix.GetVnlMatrix()[1][0] * matrix.GetVnlMatrix()[1][0]
                           + matrix.GetVnlMatrix()[1][1] * matrix.GetVnlMatrix()[1][1]
                           + matrix.GetVnlMatrix()[1][2] * matrix.GetVnlMatrix()[1][2] ) );
  m_Scale[2] = sqrt( fabs( matrix.GetVnlMatrix()[2][0] * matrix.GetVnlMatrix()[2][0]
                           + matrix.GetVnlMatrix()[2][1] * matrix.GetVnlMatrix()[2][1]
                           + matrix.GetVnlMatrix()[2][2] * matrix.GetVnlMatrix()[2][2] ) );

  MatrixType scaleMatrix;
  scaleMatrix(0, 0) = m_Scale[0];
  scaleMatrix(1, 1) = m_Scale[1];
  scaleMatrix(2, 2) = m_Scale[2];
  scaleMatrix = scaleMatrix.GetInverse();
  matrix = matrix * scaleMatrix;

  VersorType v;
  v.Set( matrix );
  this->SetVarVersor( v );
}

// Print self
template <class TScalarType>
void
AnisotropicSimilarity3DTransform<TScalarType>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Scale = " << m_Scale << std::endl;
}

} // namespace

#endif
