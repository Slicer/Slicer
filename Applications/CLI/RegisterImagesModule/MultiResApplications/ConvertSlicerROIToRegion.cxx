/*=========================================================================

  Program:   Slicer
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

  =========================================================================*/

#include <itkImageBase.h>
#include <itkSlicerBoxSpatialObject.h>


itk::ImageRegion<3>
convertPointsToRegion(const itk::Point<double, 3>& p1,
                      const itk::Point<double, 3>& p2,
                      const itk::ImageBase<3>* img)
{
    // convert two points to an ITK region
    itk::Index<3> ind1, ind2;
    itk::Size<3> size;

    img->TransformPhysicalPointToIndex(p1, ind1);
    img->TransformPhysicalPointToIndex(p2, ind2);
    
    // Find the absolute size of the bounding region
    size[0] = std::abs(ind1[0] - ind2[0]);
    size[1] = std::abs(ind1[1] - ind2[1]);
    size[2] = std::abs(ind1[2] - ind2[2]);
    
    itk::Index<3> startind;
    startind[0] = std::min(ind1[0], ind2[0]);
    startind[1] = std::min(ind1[1], ind2[1]);
    startind[2] = std::min(ind1[2], ind2[2]);

    return itk::ImageRegion<3>(startind, size);   
}

itk::SlicerBoxSpatialObject<3>::Pointer
convertPointsToBoxSpatialObject(const itk::Point<double, 3>& p1,
                                const itk::Point<double, 3>& p2)
{
  itk::SlicerBoxSpatialObject<3>::SizeType size;
  size[0] = std::fabs(p2[0] - p1[0]);
  size[1] = std::fabs(p2[1] - p1[1]);
  size[2] = std::fabs(p2[2] - p1[2]);

  itk::SlicerBoxSpatialObject<3>::Pointer box = 
    itk::SlicerBoxSpatialObject<3>::New();

  box->SetSize(size);

  itk::SlicerBoxSpatialObject<3>::TransformType::OffsetType off;
  off[0] = std::min(p1[0], p2[0]);
  off[1] = std::min(p1[1], p2[1]);
  off[2] = std::min(p1[2], p2[2]);
  
  box->GetObjectToParentTransform()->SetOffset(off);
  box->ComputeObjectToWorldTransform();
  box->ComputeBoundingBox();

  return box;
}

