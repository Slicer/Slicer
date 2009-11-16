/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEulerSimilarity3DTransform.txx,v $
  Language:  C++
  Date:      $Date: 2009-03-03 15:09:08 $
  Version:   $Revision: 1.17 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkEulerSimilarity3DTransform_txx
#define __itkEulerSimilarity3DTransform_txx

#include "itkEulerSimilarity3DTransform.h"


namespace itk
{

// Constructor with default arguments
template <class TScalarType>
EulerSimilarity3DTransform<TScalarType>
::EulerSimilarity3DTransform() :
  Superclass(OutputSpaceDimension, ParametersDimension)
{
  m_Scale = 1.0;
}


// Constructor with arguments
template<class TScalarType>
EulerSimilarity3DTransform<TScalarType>::
EulerSimilarity3DTransform( unsigned int spaceDimension, 
                            unsigned int parametersDimension):
  Superclass(spaceDimension, parametersDimension)
{
  m_Scale = 1.0;
}

// Constructor with arguments
template<class TScalarType>
EulerSimilarity3DTransform<TScalarType>::
EulerSimilarity3DTransform( const MatrixType & matrix,
                            const OutputVectorType & offset):
  Superclass(matrix, offset)
{
  this->ComputeMatrixParameters();
}

// Directly set the matrix
template<class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>
::SetMatrix( const MatrixType & matrix )
{
  // Any matrix should work - bypass orthogonality testing
  typedef MatrixOffsetTransformBase<TScalarType, 3> Baseclass;
  this->Baseclass::SetMatrix( matrix );
} 

// Set Parameters
template <class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>
::SetParameters( const ParametersType & parameters )
{

  itkDebugMacro( << "Setting parameters " << parameters );

  // Matrix must be defined before translation so that offset can be computed
  // from translation
  
  // Transfer the versor part
  this->SetVarRotation(parameters[0],
                       parameters[1],
                       parameters[2]);

  m_Scale = parameters[6];

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

  itkDebugMacro(<<"After setting parameters ");
}

//
// Get Parameters
// 
// Parameters are ordered as:
//
// p[0:2] = Euler angles in X, Y, and Z
// p[3:5] = translation components
// p[6:8] = Scale
//

template <class TScalarType>
const typename EulerSimilarity3DTransform<TScalarType>::ParametersType &
EulerSimilarity3DTransform<TScalarType>
::GetParameters( void ) const
{
  itkDebugMacro( << "Getting parameters ");

  this->m_Parameters[0] = this->GetAngleX();
  this->m_Parameters[1] = this->GetAngleY();
  this->m_Parameters[2] = this->GetAngleZ();

  this->m_Parameters[3] = this->GetTranslation()[0];
  this->m_Parameters[4] = this->GetTranslation()[1];
  this->m_Parameters[5] = this->GetTranslation()[2];

  this->m_Parameters[6] = this->m_Scale;

  itkDebugMacro(<<"After getting parameters " << this->m_Parameters );

  return this->m_Parameters;
}

template <class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>
::SetIdentity()
{
  m_Scale = 1.0;
  Superclass::SetIdentity();
}

template <class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>
::SetScale( TScalarType scale )
{
  this->m_Scale = scale;
  this->ComputeMatrix();
}

template <class TScalarType>
TScalarType
EulerSimilarity3DTransform<TScalarType>
::GetScale() const
{
  return this->m_Scale;
}

// Compute the matrix
template <class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>
::ComputeMatrix( void )
{
  Superclass::ComputeMatrix();

  MatrixType rotation = this->GetMatrix();

  rotation *= m_Scale;

  this->SetVarMatrix ( rotation );
}

template <class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>
::ComputeMatrixParameters( void )
{
  itkExceptionMacro( << "Setting the matrix of a EulerSimilarity3D transform is not supported at this time." );
}

template <class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>
::SetVarScale(TScalarType s)
{
  this->m_Scale = s;
}

// Print self
template<class TScalarType>
void
EulerSimilarity3DTransform<TScalarType>::
PrintSelf(std::ostream &os, Indent indent) const
{

  Superclass::PrintSelf(os,indent);
  
  os << indent << "Scale:       " << m_Scale        << std::endl;
}

// Set parameters
template<class TScalarType>
const typename EulerSimilarity3DTransform<TScalarType>::JacobianType &
EulerSimilarity3DTransform<TScalarType>::
GetJacobian( const InputPointType & p ) const
{
  // need to check if angles are in the right order
  const double cx = vcl_cos(this->GetAngleX());
  const double sx = vcl_sin(this->GetAngleX());
  const double cy = vcl_cos(this->GetAngleY());
  const double sy = vcl_sin(this->GetAngleY()); 
  const double cz = vcl_cos(this->GetAngleZ());
  const double sz = vcl_sin(this->GetAngleZ());
  const double ss = this->GetScale();

  this->m_Jacobian.Fill(0.0);

  const InputVectorType pp = p - this->GetCenter();

  const double px = p[0] - this->GetCenter()[0];
  const double py = p[1] - this->GetCenter()[1];
  const double pz = p[2] - this->GetCenter()[2];

  this->m_Jacobian[0][0] = ss*((-sz*cx*sy)*px + (sz*sx)*py + (sz*cx*cy)*pz);
  this->m_Jacobian[1][0] = ss*((cz*cx*sy)*px + (-cz*sx)*py + (-cz*cx*cy)*pz);
  this->m_Jacobian[2][0] = ss*((sx*sy)*px + (cx)*py + (-sx*cy)*pz);  
    
  this->m_Jacobian[0][1] = ss*((-cz*sy-sz*sx*cy)*px + (cz*cy-sz*sx*sy)*pz);
  this->m_Jacobian[1][1] = ss*((-sz*sy+cz*sx*cy)*px + (sz*cy+cz*sx*sy)*pz);
  this->m_Jacobian[2][1] = ss*((-cx*cy)*px + (-cx*sy)*pz);
    
  this->m_Jacobian[0][2] = ss*((-sz*cy-cz*sx*sy)*px + (-cz*cx)*py 
                               + (-sz*sy+cz*sx*cy)*pz);
  this->m_Jacobian[1][2] = ss*((cz*cy-sz*sx*sy)*px + (-sz*cx)*py 
                               + (cz*sy+sz*sx*cy)*pz);
  this->m_Jacobian[2][2] = 0;
 
  // compute derivatives for the translation part
  unsigned int blockOffset = 3;  
  for(unsigned int dim=0; dim < SpaceDimension; dim++ ) 
    {
    this->m_Jacobian[ dim ][ blockOffset + dim ] = 1.0;
    }

  // compute Jacobian with respect to the scale parameter

  this->m_Jacobian[0][6] = ((cz*cy-sz*sx*sy)*px
                            + (-sz*cx)*py 
                            + (cz*sy+sz*sx*cy)*pz);
  this->m_Jacobian[1][6] = ((sz*cy+cz*sx*sy)*px
                            + (cz*cx)*py 
                            + (sz*sy-cz*sx*cy)*pz);
  this->m_Jacobian[2][6] = ((-cx*sy)*px
                            + sx*py
                            + (cx*cy)*pz);

  return this->m_Jacobian; 
}

} // namespace

#endif
