/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

// .NAME vtkITKIslandMath - ITK-based utilities for manipulating connected regions in label maps
// .SECTION Description
// vtkITKIslandMath


#ifndef __vtkITKIslandMath_h
#define __vtkITKIslandMath_h


#include "vtkITK.h"
#include "vtkSimpleImageToImageFilter.h"

class VTK_ITK_EXPORT vtkITKIslandMath : public vtkSimpleImageToImageFilter
{
 public:
  static vtkITKIslandMath *New();
  vtkTypeRevisionMacro(vtkITKIslandMath, vtkSimpleImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // If non-zero, islands are defined by pixels that touch on edges and/or vertices.
  // If zero, pixels are only considered part of the same island if their faces/edges touch
  vtkGetMacro(FullyConnected, int);
  vtkSetMacro(FullyConnected, int);

  // Description:
  // Minimum island size (in pixels).  Islands smaller than this are ignored.
  vtkGetMacro(MinimumSize, vtkIdType);
  vtkSetMacro(MinimumSize, vtkIdType);

  // Description:
  // Maximum island size (in pixels).  Islands larger than this are ignored.
  vtkGetMacro(MaximumSize, vtkIdType);
  vtkSetMacro(MaximumSize, vtkIdType);

  // Description:
  // TODO: Not yet implemented
  // If zero, islands are defined by 3D connectivity
  // If non-zero, islands are evaluated in a sequence of 2D planes
  // (IJ=3, IK=2, JK=1)
  vtkGetMacro(SliceBySlice, int);
  vtkSetMacro(SliceBySlice, int);
  void SetSliceBySliceToIJ() {this->SetSliceBySlice(3);}
  void SetSliceBySliceToIK() {this->SetSliceBySlice(2);}
  void SetSliceBySliceToJK() {this->SetSliceBySlice(1);}



protected:
  vtkITKIslandMath();
  ~vtkITKIslandMath();

  virtual void SimpleExecute(vtkImageData* input, vtkImageData* output);

  int FullyConnected;
  int SliceBySlice;
  vtkIdType MinimumSize;
  vtkIdType MaximumSize;
  
private:
  vtkITKIslandMath(const vtkITKIslandMath&);  // Not implemented.
  void operator=(const vtkITKIslandMath&);  // Not implemented.
};

#endif




