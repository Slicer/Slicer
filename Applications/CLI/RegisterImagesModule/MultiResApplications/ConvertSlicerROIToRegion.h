/*=========================================================================

  Program:   Slicer
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================*/

#ifndef ConvertSlicerROIToRegion_h_
#define ConvertSlicerROIToRegion_h_

#include "vtkRegisterImagesModuleMultiResApplicationsWin32Header.h"
#include "itkSlicerBoxSpatialObject.h"

#include <itkRegion.h>
#include <itkPoint.h>
#include <itkImageBase.h>

#include <vector>

VTK_REGISTERIMAGESMODULEMULTIRESAPPLICATIONS_EXPORT
itk::ImageRegion<3> convertPointsToRegion(const itk::Point<double, 3>& p1, const itk::Point<double, 3>& p2,
                                          const itk::ImageBase<3>* img);

VTK_REGISTERIMAGESMODULEMULTIRESAPPLICATIONS_EXPORT
itk::SlicerBoxSpatialObject<3>::Pointer convertPointsToBoxSpatialObject(const itk::Point<double,
                                                                                         3>& p1,
                                                                        const itk::Point<double,
                                                                                         3> &
                                                                        p2);

#endif
