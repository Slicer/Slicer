/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DTransform_txx
#define itkDiffusionTensor3DTransform_txx

#include "itkDiffusionTensor3DTransform.h"

namespace itk
{

template <class TData>
DiffusionTensor3DTransform<TData>
::DiffusionTensor3DTransform()
{
  // Initialize the Measurement Frame to Identity
  m_MeasurementFrame.SetIdentity();
}

} // end namespace itk
#endif
