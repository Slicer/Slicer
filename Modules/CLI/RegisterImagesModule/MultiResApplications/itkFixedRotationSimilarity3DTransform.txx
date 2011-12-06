/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFixedRotationSimilarity3DTransform.txx,v $
  Language:  C++
  Date:      $Date: 2007/11/27 16:04:48 $
  Version:   $Revision: 1.9 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkFixedRotationSimilarity3DTransform_txx
#define _itkFixedRotationSimilarity3DTransform_txx

#include "itkFixedRotationSimilarity3DTransform.h"
#include "vnl/vnl_math.h"
#include "vnl/vnl_det.h"

namespace itk
{

// Constructor with default arguments
template <class TScalarType>
FixedRotationSimilarity3DTransform<TScalarType>
::FixedRotationSimilarity3DTransform() :
#if ITK_VERSION_MAJOR >= 4
  Superclass(ParametersDimension)
#else
  Superclass(OutputSpaceDimension, ParametersDimension)
#endif
{
  this->SetScale(1.0);
}

// Constructor with arguments
template <class TScalarType>
FixedRotationSimilarity3DTransform<TScalarType>::FixedRotationSimilarity3DTransform( unsigned int paramDim) :
#if ITK_VERSION_MAJOR >= 4
  Superclass(paramDim)
#else
  Superclass(OutputSpaceDimension, paramDim)
#endif
{
  this->SetScale(1.0);
}

// Constructor with arguments
template <class TScalarType>
FixedRotationSimilarity3DTransform<TScalarType>::FixedRotationSimilarity3DTransform( const MatrixType & matrix,
                                                                                     const OutputVectorType & offset) :
  Superclass(matrix, offset)
{
  this->SetScale(1.0);
}

// Set Parameters
template <class TScalarType>
void
FixedRotationSimilarity3DTransform<TScalarType>
::SetParameters( const ParametersType & parameters )
{

  itkDebugMacro( << "Setting parameters " << parameters );

  // Set the scale
  this->SetScale(parameters[3]); // must be set before calling ComputeMatrix();
  this->ComputeMatrix();

  // Transfer the translation part
  TranslationType newTranslation;
  newTranslation[0] = parameters[0];
  newTranslation[1] = parameters[1];
  newTranslation[2] = parameters[2];
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
// p[0:2] = trasnlation
// p[3]   = uniform scale
//

template <class TScalarType>
const typename FixedRotationSimilarity3DTransform<TScalarType>::ParametersType
& FixedRotationSimilarity3DTransform<TScalarType>
::GetParameters( void ) const
  {
  itkDebugMacro( << "Getting parameters ");

  // Transfer the translation
  this->m_Parameters[0] = this->GetTranslation()[0];
  this->m_Parameters[1] = this->GetTranslation()[1];
  this->m_Parameters[2] = this->GetTranslation()[2];

  this->m_Parameters[3] = this->GetScale();

  itkDebugMacro(<< "After getting parameters " << this->m_Parameters );

  return this->m_Parameters;
  }

template <class TScalarType>
void
FixedRotationSimilarity3DTransform<TScalarType>
::SetIdentity( void )
{
  Superclass::SetIdentity();
  this->SetScale(1.0);
}

// Set parameters
template <class TScalarType>
const typename FixedRotationSimilarity3DTransform<TScalarType>::JacobianType
& FixedRotationSimilarity3DTransform<TScalarType>::
GetJacobian( const InputPointType &p ) const
  {
  ComputeJacobianWithRespectToParameters( p, this->m_NonThreadsafeSharedJacobian );
  return this->m_NonThreadsafeSharedJacobian;
  }

template <class TScalarType>
void
FixedRotationSimilarity3DTransform<TScalarType>::ComputeJacobianWithRespectToParameters( const InputPointType & p,
                                                                                         JacobianType & jacobian )
const
{
  jacobian.SetSize( OutputSpaceDimension, ParametersDimension );
  jacobian.Fill(0.0);

  const InputVectorType pp = p - this->GetCenter();

  // compute Jacobian with respect to the translation parameters
  jacobian[0][0] = 1.0;
  jacobian[1][1] = 1.0;
  jacobian[2][2] = 1.0;

  // compute Jacobian with respect to the scale parameter
  const MatrixType & matrix = this->GetMatrix();

  // Applies rotation and scale
  const InputVectorType mpp = matrix * pp;

  // Remove back out the scale
  jacobian[0][3] = mpp[0] / this->GetScale();
  jacobian[1][3] = mpp[1] / this->GetScale();
  jacobian[2][3] = mpp[2] / this->GetScale();
}

// Print self
template <class TScalarType>
void
FixedRotationSimilarity3DTransform<TScalarType>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Scale = " << this->GetScale() << std::endl;
}

} // namespace

#endif
