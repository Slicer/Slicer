/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEulerAnisotropicSimilarity3DTransform.txx,v $
  Language:  C++
  Date:      $Date: 2009-03-03 15:09:08 $
  Version:   $Revision: 1.17 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkEulerAnisotropicSimilarity3DTransform_txx
#define __itkEulerAnisotropicSimilarity3DTransform_txx

#include "itkEulerAnisotropicSimilarity3DTransform.h"

namespace itk
{

// Constructor with default arguments
template <class TScalarType>
EulerAnisotropicSimilarity3DTransform<TScalarType>
::EulerAnisotropicSimilarity3DTransform() :
#if ITK_VERSION_MAJOR >= 4
  Superclass(ParametersDimension)
#else
  Superclass(OutputSpaceDimension, ParametersDimension)
#endif
{
  m_Scale.Fill( 1.0 );
}

// Constructor with arguments
template <class TScalarType>
EulerAnisotropicSimilarity3DTransform<TScalarType>::EulerAnisotropicSimilarity3DTransform(
  unsigned int parametersDimension) :
#if ITK_VERSION_MAJOR >= 4
  Superclass(parametersDimension)
#else
  Superclass(OutputSpaceDimension, parametersDimension)
#endif
{
  m_Scale.Fill( 1.0 );
}

// Constructor with arguments
template <class TScalarType>
EulerAnisotropicSimilarity3DTransform<TScalarType>::EulerAnisotropicSimilarity3DTransform(
  const MatrixType & matrix, const OutputVectorType & offset) :
  Superclass(matrix, offset)
{
  this->ComputeMatrixParameters();
}

// Directly set the matrix
template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>
::SetMatrix( const MatrixType & matrix )
{
  // Any matrix should work - bypass orthogonality testing
  typedef MatrixOffsetTransformBase<TScalarType, 3> Baseclass;
  this->Baseclass::SetMatrix( matrix );
}

// Set Parameters
template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>
::SetParameters( const ParametersType & parameters )
{

  itkDebugMacro( << "Setting parameters " << parameters );

  // Transfer the versor part
  this->SetVarRotation(parameters[0],
                       parameters[1],
                       parameters[2]);

  // Matrix must be defined before translation so that offset can be computed
  // from translation
  m_Scale[0] = parameters[6];
  m_Scale[1] = parameters[7];
  m_Scale[2] = parameters[8];

  this->ComputeMatrix();

  OutputVectorType newTranslation;
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
// p[0:2] = right part of the versor (axis times vcl_sin(t/2))
// p[3:5] = translation components
// p[6:8] = Scale
//

template <class TScalarType>
const typename EulerAnisotropicSimilarity3DTransform<TScalarType>::ParametersType
& EulerAnisotropicSimilarity3DTransform<TScalarType>
::GetParameters( void ) const
  {
  itkDebugMacro( << "Getting parameters ");

  this->m_Parameters[0] = this->GetAngleX();
  this->m_Parameters[1] = this->GetAngleY();
  this->m_Parameters[2] = this->GetAngleZ();

  this->m_Parameters[3] = this->GetTranslation()[0];
  this->m_Parameters[4] = this->GetTranslation()[1];
  this->m_Parameters[5] = this->GetTranslation()[2];

  this->m_Parameters[6] = this->GetScale()[0];
  this->m_Parameters[7] = this->GetScale()[1];
  this->m_Parameters[8] = this->GetScale()[2];

  itkDebugMacro(<< "After getting parameters " << this->m_Parameters );

  return this->m_Parameters;
  }

template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>
::SetIdentity()
{
  m_Scale.Fill( 1.0 );
  Superclass::SetIdentity();
}

template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>
::SetScale( const ScaleVectorType & scale )
{
  m_Scale = scale;
  this->ComputeMatrix();
}

// Compute the matrix
template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>
::ComputeMatrix( void )
{
  Superclass::ComputeMatrix();

  MatrixType rotation = this->GetMatrix();

  MatrixType scaleMatrix;
  scaleMatrix.SetIdentity();
  scaleMatrix[0][0] = m_Scale[0];
  scaleMatrix[1][1] = m_Scale[1];
  scaleMatrix[2][2] = m_Scale[2];

  // Is this the correct?
  this->SetVarMatrix( rotation * scaleMatrix );
}

template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>
::ComputeMatrixParameters( void )
{
  itkExceptionMacro( << "Setting the matrix of a EulerAnisotropicSimilarity3D transform is not supported at this time." );
}

// Print self
template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>::PrintSelf(std::ostream & os, Indent indent) const
{

  Superclass::PrintSelf(os, indent);

  os << indent << "Scale:       " << m_Scale        << std::endl;
}

// Set parameters
template <class TScalarType>
const typename EulerAnisotropicSimilarity3DTransform<TScalarType>::JacobianType
& EulerAnisotropicSimilarity3DTransform<TScalarType>::
GetJacobian( const InputPointType &p ) const
  {
  ComputeJacobianWithRespectToParameters( p, this->m_NonThreadsafeSharedJacobian );
  return this->m_NonThreadsafeSharedJacobian;
  }

template <class TScalarType>
void
EulerAnisotropicSimilarity3DTransform<TScalarType>::ComputeJacobianWithRespectToParameters( const InputPointType & p,
                                                                                            JacobianType & jacobian )
const
{
  jacobian.SetSize( OutputSpaceDimension, ParametersDimension );
  jacobian.Fill(0.0);

  const InputVectorType pp = p - this->GetCenter();

  const double px = pp[0];
  const double py = pp[1];
  const double pz = pp[2];

  const double scx = m_Scale[0];
  const double scy = m_Scale[1];
  const double scz = m_Scale[2];

  const double x = this->GetAngleX();
  const double y = this->GetAngleY();
  const double z = this->GetAngleZ();

  // Computed using Maxima
  // Rotation jacobian
  jacobian[0][0] = -px *scx *                                     cos(x) * sin(y) * sin(z) + pz *scz *cos(x) * cos(y)
    * sin(z) + py *scy *                                          sin(x) * sin(z);
  jacobian[1][0] = px * scx * cos(x) * sin(y) * cos(z) - pz *scz *cos(x) * cos(y) * cos(z) - py *scy *sin(x) * cos(z);
  jacobian[2][0] = px * scx * sin(x) * sin(y) - pz *scz *         sin(x) * cos(y) + py *scy *cos(x);

  jacobian[0][1] = pz * scz
    * (cos(y) * cos(z) - sin(x) * sin(y) * sin(z) ) + px * scx * (-sin(x) * cos(y) * sin(z) - sin(y) * cos(z) );
  jacobian[1][1] = px * scx
    * (sin(x) * cos(y) * cos(z) - sin(y) * sin(z) ) + pz * scz * (cos(y) * sin(z) + sin(x) * sin(y) * cos(z) );
  jacobian[2][1] = -pz *scz *cos(x) * sin(y) - px *scx *cos(x) * cos(y);

  jacobian[0][2] = pz * scz
    * (sin(x) * cos(y) * cos(z) - sin(y)
       * sin(z) ) + px * scx * (-cos(y) * sin(z) - sin(x) * sin(y) * cos(z) ) - py *scy *cos(x) * cos(z);
  jacobian[1][2] = px * scx
    * (cos(y) * cos(z) - sin(x) * sin(y)
       * sin(z) ) + pz * scz * (sin(x) * cos(y) * sin(z) + sin(y) * cos(z) ) - py *scy *cos(x) * sin(z);
  jacobian[2][2] = 0.0;

  // Translation jacobian
  jacobian[0][3] = 1.0;
  jacobian[1][4] = 1.0;
  jacobian[2][5] = 1.0;

  // Scaling jacobian
  // Scale_x
  jacobian[0][6] = px * (cos(y) * cos(z) - sin(x) * sin(y) * sin(z) );
  jacobian[1][6] = px * (cos(y) * sin(z) + sin(x) * sin(y) * cos(z) );
  jacobian[2][6] = -px *cos(x) * sin(y);

  // Scale_y
  jacobian[0][7] = -py *cos(x) * sin(z);
  jacobian[1][7] = py * cos(x) * cos(z);
  jacobian[2][7] = py * sin(x);

  // Scale_z
  jacobian[0][8] = pz * (sin(x) * cos(y) * sin(z) + sin(y) * cos(z) );
  jacobian[1][8] = pz * (sin(y) * sin(z) - sin(x) * cos(y) * cos(z) );
  jacobian[2][8] = pz * cos(x) * cos(y);
}

} // namespace

#endif
