/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifndef __vtkITKIslandMath_h
#define __vtkITKIslandMath_h

#include "vtkITK.h"
#include "vtkSimpleImageToImageFilter.h"

/// \brief ITK-based utilities for manipulating connected regions in label maps.
/// Limitation: The filter does not work correctly with input volume that has
/// unsigned long scalar type on Linux and MacOSX.
///
class VTK_ITK_EXPORT vtkITKIslandMath : public vtkSimpleImageToImageFilter
{
 public:
  static vtkITKIslandMath *New();
  vtkTypeMacro(vtkITKIslandMath, vtkSimpleImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///
  /// If non-zero, islands are defined by pixels that touch on edges and/or vertices.
  /// If zero, pixels are only considered part of the same island if their faces/edges touch
  vtkGetMacro(FullyConnected, int);
  vtkSetMacro(FullyConnected, int);

  ///
  /// Minimum island size (in pixels).  Islands smaller than this are ignored.
  vtkGetMacro(MinimumSize, vtkIdType);
  vtkSetMacro(MinimumSize, vtkIdType);

  ///
  /// Maximum island size (in pixels).  Islands larger than this are ignored.
  vtkGetMacro(MaximumSize, vtkIdType);
  vtkSetMacro(MaximumSize, vtkIdType);

  ///
  /// TODO: Not yet implemented
  /// If zero, islands are defined by 3D connectivity
  /// If non-zero, islands are evaluated in a sequence of 2D planes
  /// (IJ=3, IK=2, JK=1)
  vtkGetMacro(SliceBySlice, int);
  vtkSetMacro(SliceBySlice, int);
  void SetSliceBySliceToIJ() {this->SetSliceBySlice(3);}
  void SetSliceBySliceToIK() {this->SetSliceBySlice(2);}
  void SetSliceBySliceToJK() {this->SetSliceBySlice(1);}

  ///
  /// Accessors to describe result of calculations
  vtkGetMacro(NumberOfIslands, unsigned long);
  vtkSetMacro(NumberOfIslands, unsigned long);
  vtkGetMacro(OriginalNumberOfIslands, unsigned long);
  vtkSetMacro(OriginalNumberOfIslands, unsigned long);


protected:
  vtkITKIslandMath();
  ~vtkITKIslandMath() override;

  void SimpleExecute(vtkImageData* input, vtkImageData* output) override;

  int FullyConnected;
  int SliceBySlice;
  vtkIdType MinimumSize;
  vtkIdType MaximumSize;

  unsigned long NumberOfIslands;
  unsigned long OriginalNumberOfIslands;

private:
  vtkITKIslandMath(const vtkITKIslandMath&) = delete;
  void operator=(const vtkITKIslandMath&) = delete;
};

#endif
