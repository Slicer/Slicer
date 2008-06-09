/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageSlicePaint.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageSlicePaint - Extract or Replace an arbitrary bilinear
// subvolume of an image and apply a paint operation to the subvolume
//
// .SECTION Description
// vtkImageSlicePaint take the given TopLeft, TopRight, and BottomRight, 
// and BottomLeft points to form a bilinear surface within the volume.
// If the ExtractImage is set, then it will be filled with copies of all
// pixels from this surface.
//
// If the ReplaceImage is set, then it will be used to replace the pixels
// of this surface.
//
// The expected use is that the filter will be run once to get the ExtractImage
// which will then be processed (e.g. a paint brush applied) and then 
// the image will be put back with the ReplaceImage.  The unmodified version
// of the ExtractImage can be saved together with the original coordinates
// in order to implement undo.
//
// The WorkingImage is modified using either a round paintbrush or an
// image mask.  In either case, as the region is being traversed in IJK space,
// the coordinate is transformed to World space to check if it should be modified
// or not.  If it is a candidate for modification, then the WorkingImage will be set
// to the PaintLabel value under the following conditions:
//   PaintOver : always draw if true, only draw on non-zero Working if false
//   ThresholdPaint : Only draw if BackgroundImage is within ThresholdPaint range
//

#ifndef __vtkImageSlicePaint_h
#define __vtkImageSlicePaint_h

#include "vtkSlicerBaseLogic.h"

#include "vtkImageInPlaceFilter.h"

#include "vtkImageData.h"
#include "vtkMatrix4x4.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkImageSlicePaint : public vtkObject
{
public:
  static vtkImageSlicePaint *New();
  vtkTypeRevisionMacro(vtkImageSlicePaint,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // Sets/Gets the PaintRegion in IJK (pixel) coordinates of the Working image
  vtkSetVector3Macro(TopLeft, int);
  vtkGetVector3Macro(TopLeft, int);
  vtkSetVector3Macro(TopRight, int);
  vtkGetVector3Macro(TopRight, int);
  vtkSetVector3Macro(BottomLeft, int);
  vtkGetVector3Macro(BottomLeft, int);
  vtkSetVector3Macro(BottomRight, int);
  vtkGetVector3Macro(BottomRight, int);

  // Description:
  // The center and radius of the brush are in world space 
  // - ijk coordinates are mapped to work to see of pixels
  //   are within the brush before applying the threshold/paint operation
  vtkSetVector3Macro(BrushCenter, double);
  vtkGetVector3Macro(BrushCenter, double);
  vtkSetMacro(BrushRadius, double);
  vtkGetMacro(BrushRadius, double);

  // Description:
  // The mask image: used instead of brush if non NULL
  // - image corresponds to the PaintRegion but is
  //   in World coordinates.
  vtkSetObjectMacro(MaskImage, vtkImageData);
  vtkGetObjectMacro(MaskImage, vtkImageData);

  // Description:
  // The reference image for threshold calculations
  vtkSetObjectMacro(BackgroundImage, vtkImageData);
  vtkGetObjectMacro(BackgroundImage, vtkImageData);

  // Description:
  // Image data to be painted into
  vtkSetObjectMacro(WorkingImage, vtkImageData);
  vtkGetObjectMacro(WorkingImage, vtkImageData);

  // Description:
  // The place to store data pulled out
  vtkSetObjectMacro(ExtractImage, vtkImageData);
  vtkGetObjectMacro(ExtractImage, vtkImageData);

  // Description:
  // The place to get data to be replaced
  vtkSetObjectMacro(ReplaceImage, vtkImageData);
  vtkGetObjectMacro(ReplaceImage, vtkImageData);

  // Description:
  // matrices to map from voxel coordinates (IJK) to world
  vtkSetObjectMacro(BackgroundIJKToWorld, vtkMatrix4x4);
  vtkGetObjectMacro(BackgroundIJKToWorld, vtkMatrix4x4);
  vtkSetObjectMacro(WorkingIJKToWorld, vtkMatrix4x4);
  vtkGetObjectMacro(WorkingIJKToWorld, vtkMatrix4x4);
  vtkSetObjectMacro(MaskIJKToWorld, vtkMatrix4x4);
  vtkGetObjectMacro(MaskIJKToWorld, vtkMatrix4x4);

  // Description:
  // PaintLabel is the value that gets painted into the Working Image
  vtkSetMacro(PaintLabel, double); 
  vtkGetMacro(PaintLabel, double); 

  // Description:
  // PaintOver mode on means that the pixel value should be set in 
  // the working image even if it is non-zero. 
  vtkSetMacro(PaintOver, int); 
  vtkGetMacro(PaintOver, int); 

  // Description:
  // ThresholdPaint mode on means check the background value and
  // only set the label map if the background is inside the range
  // (also obeys the PaintOver flag)
  vtkSetMacro(ThresholdPaint, int); 
  vtkGetMacro(ThresholdPaint, int); 

  // Description:
  // Min/Max for the ThresholdPaint mode
  vtkSetVector2Macro(ThresholdPaintRange, double);
  vtkGetVector2Macro(ThresholdPaintRange, double);

  // Description:
  // Apply the paint operation
  void Paint();
 
protected:
  vtkImageSlicePaint();
  ~vtkImageSlicePaint();

  int TopLeft[3];
  int TopRight[3];
  int BottomLeft[3];
  int BottomRight[3];

  vtkImageData *MaskImage;
  vtkImageData *BackgroundImage;
  vtkImageData *WorkingImage;
  vtkImageData *ExtractImage;
  vtkImageData *ReplaceImage;

  vtkMatrix4x4 *BackgroundIJKToWorld;
  vtkMatrix4x4 *WorkingIJKToWorld;
  vtkMatrix4x4 *MaskIJKToWorld;

  double PaintLabel;
  double BrushCenter[3]; // in World Coordinates
  double BrushRadius;
  int ThresholdPaint;
  double ThresholdPaintRange[2];
  int PaintOver;

private:
  vtkImageSlicePaint(const vtkImageSlicePaint&);  // Not implemented.
  void operator=(const vtkImageSlicePaint&);  // Not implemented.
};



#endif



