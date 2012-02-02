/*=========================================================================

  Program:   Slicer
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================*/

#ifndef ScalingHeuristics_h_
#define ScalingHeuristics_h_

#include "vtkRegisterImagesModuleMultiResApplicationsWin32Header.h"

#include <itkImageBase.h>
#include <itkVector.h>

// The goal of this is to set the scale of each parameter
// to a value such that a change in that parameter of 1.0
// results in a one-voxel movement (where the dimension of a voxel
// is defined by its smallest size).

class VTK_REGISTERIMAGESMODULEMULTIRESAPPLICATIONS_EXPORT ScalingValues
{
public:
  ScalingValues(const itk::ImageBase<3>* image, const itk::Point<double, 3> centerOfRotation);

  double TranslationScale;
  double RotationScale;
  double ScalingScale;
  double SkewingScale;
};

#endif
