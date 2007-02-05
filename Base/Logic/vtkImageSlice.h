/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageSlice.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageSlice - Slices a volume along a new set of axes.
// .SECTION Description
// vtkImageSlice uses a vtkMatrix4x4 to pull a slice out of a volume.
// This is based on David Gobbi's vtkImageReslice, but tries to be more
// efficient by only treating the special case of a linear transform
// .SECTION Caveats
// .SECTION see also
// vtkImageReslice vtkAbstractTransform vtkMatrix4x4


#ifndef __vtkImageSlice_h
#define __vtkImageSlice_h


#include "vtkAbstractTransform.h"
#include "vtkThreadedImageAlgorithm.h"

#include "vtkSlicerBaseLogic.h"

// interpolation mode constants
#define VTK_SLICE_NEAREST 0
#define VTK_SLICE_LINEAR 1
#define VTK_SLICE_CUBIC 3

class vtkImageData;
class vtkAbstractTransform;
class vtkMatrix4x4;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkImageSlice : public vtkThreadedImageAlgorithm
{
public:
  static vtkImageSlice *New();
  vtkTypeRevisionMacro(vtkImageSlice, vtkThreadedImageAlgorithm);

  virtual void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // this is the IJKToIJK transform.  It tells you how to get from
  // pixel space in the output image to pixel space in the input image
  vtkSetObjectMacro(SliceTransform, vtkAbstractTransform);
  vtkGetObjectMacro(SliceTransform, vtkAbstractTransform);

  // Description:
  // Set interpolation mode (default: nearest neighbor). 
  vtkSetMacro(InterpolationMode, int);
  vtkGetMacro(InterpolationMode, int);
  void SetInterpolationModeToNearestNeighbor() {
    this->SetInterpolationMode(VTK_SLICE_NEAREST); };
  void SetInterpolationModeToLinear() {
    this->SetInterpolationMode(VTK_SLICE_LINEAR); };
  void SetInterpolationModeToCubic() {
    this->SetInterpolationMode(VTK_SLICE_CUBIC); };
  const char *GetInterpolationModeAsString();

  // Description:
  // Set the background color (for multi-component images).
  vtkSetVector4Macro(BackgroundColor, double);
  vtkGetVector4Macro(BackgroundColor, double);

  // Description:
  // Set background grey level (for single-component images).
  void SetBackgroundLevel(double v) { this->SetBackgroundColor(v,v,v,v); };
  double GetBackgroundLevel() { return this->GetBackgroundColor()[0]; };

  // Description:
  // Set the spacing to be set in the output image.  It is not used 
  // in the calculation.  Default is 1, 1, 1.
  vtkSetVector3Macro(OutputSpacing, double);
  vtkGetVector3Macro(OutputSpacing, double);
  void SetOutputSpacingToDefault() {
    this->SetOutputSpacing(1., 1., 1.); };

  // Description:
  // Set the origin to be set in the output image.  It is not used 
  // in the calculation.  Default is 0, 0, 0
  vtkSetVector3Macro(OutputOrigin, double);
  vtkGetVector3Macro(OutputOrigin, double);
  void SetOutputOriginToDefault() {
    this->SetOutputOrigin(0., 0., 0.); };

  // Description:
  // Set the Dimensions for the output data.  
  // This defines the pixels to be calculated by the filter.
  vtkSetVector3Macro(OutputDimensions, int);
  vtkGetVector3Macro(OutputDimensions, int);

  // Description: 
  // customized MTime calculation that takes into account the input transform
  unsigned long int GetMTime();

  // Description:
  // Convenient methods for switching between nearest-neighbor and linear
  // interpolation.  
  // InterpolateOn() is equivalent to SetInterpolationModeToLinear() and
  // InterpolateOff() is equivalent to SetInterpolationModeToNearestNeighbor().
  // You should not use these methods if you use the SetInterpolationMode
  // methods.
  void SetInterpolate(int t) {
    if (t && !this->GetInterpolate()) {
      this->SetInterpolationModeToLinear(); }
    else if (!t && this->GetInterpolate()) {
      this->SetInterpolationModeToNearestNeighbor(); } };
  void InterpolateOn() {
    this->SetInterpolate(1); };
  void InterpolateOff() {
    this->SetInterpolate(0); };
  int GetInterpolate() {
    return (this->GetInterpolationMode() != VTK_SLICE_NEAREST); };


protected:
  vtkImageSlice();
  ~vtkImageSlice();

  vtkAbstractTransform *SliceTransform;
  int InterpolationMode;
  double BackgroundColor[4];
  double OutputOrigin[3];
  double OutputSpacing[3];
  int OutputDimensions[3];

  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
                                 vtkInformationVector *);
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
                                  vtkInformationVector *);
  virtual void ThreadedRequestData(vtkInformation *request,
                                   vtkInformationVector **inputVector,
                                   vtkInformationVector *outputVector,
                                   vtkImageData ***inData,
                                   vtkImageData **outData, int ext[6], int id);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

private:
  vtkImageSlice(const vtkImageSlice&);  // Not implemented.
  void operator=(const vtkImageSlice&);  // Not implemented.
};

//----------------------------------------------------------------------------
inline const char *vtkImageSlice::GetInterpolationModeAsString()
{
  switch (this->InterpolationMode)
    {
    case VTK_SLICE_NEAREST:
      return "NearestNeighbor";
    case VTK_SLICE_LINEAR:
      return "Linear";
    case VTK_SLICE_CUBIC:
      return "Cubic";
    default:
      return "";
    }
}  

#endif





