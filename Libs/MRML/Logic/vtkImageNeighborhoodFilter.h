/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#ifndef __vtkImageNeighborhoodFilter_h
#define __vtkImageNeighborhoodFilter_h

#include <vtkImageSpatialAlgorithm.h>

#include "vtkMRMLLogicExport.h"

/// \brief Augments vtkImageSpatialFilter with a mask to allow neighborhoods of any shape.
///
/// 4 or 8 neighbor connectivity, or subclasses may define their own neighborhood
/// shapes. The mask is basically a binary (0/1) filter kernel to define
/// neighbors of interest.
class VTK_MRML_LOGIC_EXPORT vtkImageNeighborhoodFilter : public vtkImageSpatialAlgorithm
{
public:
  static vtkImageNeighborhoodFilter *New();
  vtkTypeMacro(vtkImageNeighborhoodFilter,vtkImageSpatialAlgorithm);

  ///
  /// Mask that defines area of interest in the neighborhood.
  /// Value is 1 for voxels of interest, 0 otherwise.
  /// This should be used when looping through neighborhood.
  unsigned char *GetMaskPointer() {return Mask;}

  ///
  /// Use 4 or 8 neighbor connectivity in neighborhood around pixel
  /// This is in 3D
  void SetNeighborTo8();
  void SetNeighborTo4();

  ///
  /// Get the neighborhood size
  /// (for erode class)
  vtkGetMacro(Neighbor, int);

  ///
  /// Get the neighborhood extent, where the max/min numbers are
  /// offsets from the current voxel.  (So these numbers may be negative.)
  /// For use when looping through the neighborhood.
  void GetRelativeHoodExtent(int &hoodMin0, int &hoodMax0, int &hoodMin1,
                 int &hoodMax1, int &hoodMin2, int &hoodMax2);

  ///
  /// Get the increments to use when looping through the mask.
  void GetMaskIncrements(vtkIdType &maskInc0, vtkIdType &maskInc1, vtkIdType &maskInc2);

protected:
  vtkImageNeighborhoodFilter();
  ~vtkImageNeighborhoodFilter() override;

  ///
  /// set size of neighborhood of each pixel
  void SetKernelSize(int size0, int size1, int size2);

  int Neighbor;
  unsigned char *Mask;

private:
  vtkImageNeighborhoodFilter(const vtkImageNeighborhoodFilter&) = delete;
  void operator=(const vtkImageNeighborhoodFilter&) = delete;
};

#endif
